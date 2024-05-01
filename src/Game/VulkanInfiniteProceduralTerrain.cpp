#include "stdafx.h"
#include "VulkanInfiniteProceduralTerrain.h"
//-----------------------------------------------------------------------------
bool VulkanInfiniteProceduralTerrainApp::OnCreate()
{
	camera.type = Camera::CameraType::firstperson;
	camera.setPerspective(45.0f, (float)destWidth / (float)destHeight, zNear, zFar);
	camera.movementSpeed = 7.5f * 5.0f;
	camera.rotationSpeed = 0.1f;
	timerSpeed *= 0.05f;

	camera.setPosition(glm::vec3(0.0f, -25.0f, 0.0f));

	camera.update(0.0f);
	frustum.update(camera.matrices.perspective * camera.matrices.view);

	VulkanDevice::enabledFeatures.shaderClipDistance = VK_TRUE;
	VulkanDevice::enabledFeatures.samplerAnisotropy = VK_TRUE;
	VulkanDevice::enabledFeatures.depthClamp = VK_TRUE;
	VulkanDevice::enabledFeatures.fillModeNonSolid = VK_TRUE;

	VulkanDevice::enabledFeatures11.multiview = VK_TRUE;
	VulkanDevice::enabledFeatures13.dynamicRendering = VK_TRUE;

	enabledDeviceExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);

	readFileLists();

	VulkanContext::graphicsQueue = queue;
	VulkanContext::device = vulkanDevice;
	// We try to get a transfer queue for background uploads
	if (vulkanDevice->queueFamilyIndices.graphics != vulkanDevice->queueFamilyIndices.transfer) {
		std::cout << "Using dedicated transfer queue for background uploads\n";
		vkGetDeviceQueue(device, vulkanDevice->queueFamilyIndices.transfer, 0, &VulkanContext::copyQueue);
	}
	else {
		VulkanContext::copyQueue = queue;
	}

	hasExtMemoryBudget = vulkanDevice->extensionSupported("VK_EXT_memory_budget");

	loadAssets();
	prepareOffscreen();
	prepareCSM();
	setupDescriptorSetLayout();
	setupDescriptorPool();
	prepareUniformBuffers();
	createPipelines();
	setupDescriptorSet();
	loadHeightMapSettings("coastline");

	return true;
}
//-----------------------------------------------------------------------------
void VulkanInfiniteProceduralTerrainApp::OnDestroy()
{
	vkDestroySampler(device, offscreenPass.sampler, nullptr);
	// @todo: wait for detachted threads to finish (maybe use atomic active thread counter)
}
//-----------------------------------------------------------------------------
void VulkanInfiniteProceduralTerrainApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);

	if (!fixFrustum) {
		frustum.update(camera.matrices.perspective * camera.matrices.view);
	}
	// @todo
	infiniteTerrain.viewerPosition = glm::vec2(camera.position.x, camera.position.z);
	infiniteTerrain.updateVisibleChunks(frustum);
}
//-----------------------------------------------------------------------------
void VulkanInfiniteProceduralTerrainApp::OnFrame()
{
	FrameObjects currentFrame = frameObjects[getCurrentFrameIndex()];

	prepareFrame(currentFrame);

	if (stickToTerrain) {
		float h = 0.0f;
		infiniteTerrain.getHeight(camera.position, h);
		camera.position.y = h - 3.0f;
	}

	updateCascades();
	updateUniformBuffers();
	updateDrawBatches();

	updateOverlay(getCurrentFrameIndex());

	buildCommandBuffer(currentFrame.commandBuffer);

	//if (vulkanDevice->queueFamilyIndices.graphics == vulkanDevice->queueFamilyIndices.transfer) {
	//	// If we don't have a dedicated transfer queue, we need to make sure that the main and background threads don't use the (graphics) pipeline simultaneously
	//	std::lock_guard<std::mutex> guard(lock_guard);
	//}
	submitFrame(currentFrame);

	updateMemoryBudgets();

	updateHeightmap();
}
//-----------------------------------------------------------------------------
void VulkanInfiniteProceduralTerrainApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Info", nullptr, ImGuiWindowFlags_None);
	ImGui::TextUnformatted("Vulkan infinite terrain");
	ImGui::TextUnformatted("2022 by Sascha Willems");
	ImGui::TextUnformatted(deviceProperties.deviceName);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(15, 15), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_None);
	ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / lastFPS), lastFPS);
	if (overlay->header("Memory")) {
		for (int i = 0; i < memoryBudget.heapCount; i++) {
			const float divisor = 1024.0f * 1024.0f;
			ImGui::Text("Heap %i: %.2f / %.2f", i, (float)memoryBudget.heapUsage[i] / divisor, (float)memoryBudget.heapBudget[i] / divisor);
		}
	}
	if (overlay->header("Timings")) {
		ImGui::Text("Draw batch CPU: %.2f ms", profiling.drawBatchCpu.tDelta);
		ImGui::Text("Draw batch upload: %.2f ms", profiling.drawBatchUpload.tDelta);
		ImGui::Text("Draw batch total: %.2f ms", profiling.drawBatchUpdate.tDelta);
		ImGui::Text("Uniform update: %.2f ms", profiling.uniformUpdate.tDelta);
		ImGui::Text("Command buffer building: %.2f ms", profiling.cbBuild.tDelta);
	}
	ImGui::Text("Active threads: %d", activeThreadCount.load());
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Debugging", nullptr, ImGuiWindowFlags_None);
	overlay->checkBox("Fix frustum", &fixFrustum);
	overlay->checkBox("Waterplane", &displayWaterPlane);
	overlay->checkBox("Display reflection", &debugDisplayReflection);
	overlay->checkBox("Display refraction", &debugDisplayRefraction);
	overlay->checkBox("Display cascades", &cascadeDebug.enabled);
	if (cascadeDebug.enabled) {
		overlay->sliderInt("Cascade", &cascadeDebug.cascadeIndex, 0, SHADOW_MAP_CASCADE_COUNT - 1);
	}
	if (overlay->sliderFloat("Split lambda", &cascadeSplitLambda, 0.1f, 1.0f)) {
		updateCascades();
		updateUniformBuffers();
	}
	ImGui::End();

	uint32_t currentFrameIndex = getCurrentFrameIndex();

	ImGui::SetNextWindowPos(ImVec2(30, 30), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Terrain", nullptr, ImGuiWindowFlags_None);
	overlay->text("%d chunks in memory", infiniteTerrain.terrainChunks.size());
	overlay->text("%d chunks visible", infiniteTerrain.getVisibleChunkCount());
	//overlay->text("%d trees visible", infiniteTerrain.getVisibleTreeCount());
	overlay->text("%d trees visible (full)", drawBatches.trees.instanceBuffers[currentFrameIndex].elements);
	overlay->text("%d trees visible (impostor)", drawBatches.treeImpostors.instanceBuffers[currentFrameIndex].elements);
	overlay->text("%d grass patches visible", drawBatches.grass.instanceBuffers[currentFrameIndex].elements);
	int currentChunkCoordX = round((float)infiniteTerrain.viewerPosition.x / (float)(heightMapSettings.mapChunkSize - 1));
	int currentChunkCoordY = round((float)infiniteTerrain.viewerPosition.y / (float)(heightMapSettings.mapChunkSize - 1));
	overlay->text("chunk coord x = %d / y =%d", currentChunkCoordX, currentChunkCoordY);
	overlay->text("cam x = %.2f / z =%.2f", camera.position.x, camera.position.z);
	overlay->text("cam yaw = %.2f / pitch =%.2f", camera.yaw, camera.pitch);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(40, 40), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Render options", nullptr, ImGuiWindowFlags_None);
	overlay->checkBox("Shadows", &renderShadows);
	overlay->checkBox("Trees", &renderTrees);
	overlay->checkBox("Grass", &renderGrass);
	overlay->checkBox("Smooth coast line", &uniformDataParams.smoothCoastLine);
	overlay->sliderFloat("Water alpha", &uniformDataParams.waterAlpha, 1.0f, 4096.0f);
	if (overlay->sliderFloat("Chunk draw distance", &heightMapSettings.maxChunkDrawDistance, 0.0f, 1024.0f)) {
		infiniteTerrain.updateViewDistance(heightMapSettings.maxChunkDrawDistance);
	}
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Terrain layers", nullptr, ImGuiWindowFlags_None);
	for (uint32_t i = 0; i < TERRAIN_LAYER_COUNT; i++) {
		overlay->sliderFloat2(("##layer_x" + std::to_string(i)).c_str(), uniformDataParams.layers[i].x, uniformDataParams.layers[i].y, 0.0f, 1.0f);
	}
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Terrain settings", nullptr, ImGuiWindowFlags_None);
	overlay->sliderInt("Seed", &heightMapSettings.seed, 0, 128);
	overlay->sliderFloat("Noise scale", &heightMapSettings.noiseScale, 0.0f, 128.0f);
	overlay->sliderFloat("Height scale", &heightMapSettings.heightScale, 0.1f, 64.0f);
	overlay->sliderFloat("Persistence", &heightMapSettings.persistence, 0.0f, 10.0f);
	overlay->sliderFloat("Lacunarity", &heightMapSettings.lacunarity, 0.0f, 10.0f);

	ImGui::ColorEdit4("Water color", heightMapSettings.waterColor);
	ImGui::ColorEdit4("Fog color", heightMapSettings.fogColor);
	ImGui::ColorEdit4("Grass color", heightMapSettings.grassColor);

	// @todo
	//overlay->comboBox("Tree type", &heightMapSettings.treeModelIndex, treeModels);
	overlay->sliderInt("Tree density", &heightMapSettings.treeDensity, 1, 64);
	//overlay->sliderInt("Grass density", &heightMapSettings.grassDensity, 1, 512);
	overlay->sliderFloat("Min. tree size", &heightMapSettings.minTreeSize, 0.1f, heightMapSettings.maxTreeSize);
	overlay->sliderFloat("Max. tree size", &heightMapSettings.maxTreeSize, heightMapSettings.minTreeSize, 5.0f);
	overlay->comboBox("Tree type", &selectedTreeType, treeTypes);
	overlay->comboBox("Grass type", &selectedGrassType, grassTypes);
	//overlay->sliderInt("LOD", &heightMapSettings.levelOfDetail, 1, 6);
	if (overlay->button("Update heightmap")) {
		infiniteTerrain.clear();
		updateHeightmap();
	}
	if (overlay->comboBox("Load preset", &presetIndex, fileList.presets)) {
		loadHeightMapSettings(fileList.presets[presetIndex]);
	}
	if (overlay->comboBox("Terrain set", &terrainSetIndex, fileList.terrainSets)) {
		loadTerrainSet(fileList.terrainSets[terrainSetIndex]);
	}
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(70, 70), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Grass layer settings", nullptr, ImGuiWindowFlags_None);
	overlay->sliderInt("Patch dimension", &heightMapSettings.grassDim, 1, 512);
	overlay->sliderFloat("Patch scale", &heightMapSettings.grassScale, 0.25f, 2.5f);
	ImGui::End();
}
//-----------------------------------------------------------------------------
void VulkanInfiniteProceduralTerrainApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void VulkanInfiniteProceduralTerrainApp::OnKeyPressed(uint32_t key)
{
	if (key == KEY_F2)
	{
		if (camera.type == Camera::CameraType::lookat)
		{
			camera.type = Camera::CameraType::firstperson;
		}
		else
		{
			camera.type = Camera::CameraType::lookat;
		}
	}

	if (camera.type == Camera::firstperson)
	{
		switch (key)
		{
		case KEY_W:
			camera.keys.up = true;
			break;
		case KEY_S:
			camera.keys.down = true;
			break;
		case KEY_A:
			camera.keys.left = true;
			break;
		case KEY_D:
			camera.keys.right = true;
			break;
		}
	}

	if (key == KEY_F) {
		fixFrustum = !fixFrustum;
	}
	if (key == KEY_F2) {
		selectedTreeType++;
		if (selectedTreeType >= treeModelInfo.size()) {
			selectedTreeType = 0;
		}
	}
	if (key == KEY_F3) {
		renderShadows = !renderShadows;
	}
	if (key == KEY_F4) {
		renderGrass = !renderGrass;
	}
	if (key == KEY_F5) {
		renderTerrain = !renderTerrain;
	}
	if (key == KEY_F6) {
		displayWaterPlane = !displayWaterPlane;
	}
	if (key == KEY_F7) {
		stickToTerrain = !stickToTerrain;
	}
	if (key == KEY_F8) {
		std::cout << camera.position.x << " " << camera.position.y << " " << camera.position.z << "\n";
		std::cout << camera.pitch << " " << camera.yaw << "\n";
	}
}
//-----------------------------------------------------------------------------
void VulkanInfiniteProceduralTerrainApp::OnKeyUp(uint32_t key)
{
	if (camera.type == Camera::firstperson)
	{
		switch (key)
		{
		case KEY_W:
			camera.keys.up = false;
			break;
		case KEY_S:
			camera.keys.down = false;
			break;
		case KEY_A:
			camera.keys.left = false;
			break;
		case KEY_D:
			camera.keys.right = false;
			break;
		}
	}
}
//-----------------------------------------------------------------------------
void VulkanInfiniteProceduralTerrainApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
{
	if (mouseState.buttons.left)
	{
		camera.rotate(glm::vec3(dy * camera.rotationSpeed, -dx * camera.rotationSpeed, 0.0f));
	}
	if (mouseState.buttons.right)
	{
		camera.translate(glm::vec3(-0.0f, 0.0f, dy * .005f));
	}
	if (mouseState.buttons.middle)
	{
		camera.translate(glm::vec3(-dx * 0.005f, -dy * 0.005f, 0.0f));
	}
}
//-----------------------------------------------------------------------------