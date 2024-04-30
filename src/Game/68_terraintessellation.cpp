#include "stdafx.h"
#include "68_terraintessellation.h"
//-----------------------------------------------------------------------------
bool TerrainTessellationApp::OnCreate()
{
	camera.type = Camera::CameraType::firstperson;
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 512.0f);
	camera.setRotation(glm::vec3(-12.0f, 159.0f, 0.0f));
	camera.setTranslation(glm::vec3(18.0f, 22.5f, 57.5f));
	camera.movementSpeed = 10.0f;

	loadAssets();
	generateTerrain();
	if (m_physicalDevice.deviceFeatures.pipelineStatisticsQuery) {
		setupQueryResultBuffer();
	}
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void TerrainTessellationApp::OnDestroy()
{
	if (device) {
		vkDestroyPipeline(device, pipelines.terrain, nullptr);
		if (pipelines.wireframe != VK_NULL_HANDLE) {
			vkDestroyPipeline(device, pipelines.wireframe, nullptr);
		}
		vkDestroyPipeline(device, pipelines.skysphere, nullptr);

		vkDestroyPipelineLayout(device, pipelineLayouts.skysphere, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayouts.terrain, nullptr);

		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.terrain, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.skysphere, nullptr);

		uniformBuffers.skysphereVertex.destroy();
		uniformBuffers.terrainTessellation.destroy();

		textures.heightMap.destroy();
		textures.skySphere.destroy();
		textures.terrainArray.destroy();

		vkDestroyBuffer(device, terrain.vertices.buffer, nullptr);
		vkFreeMemory(device, terrain.vertices.memory, nullptr);
		vkDestroyBuffer(device, terrain.indices.buffer, nullptr);
		vkFreeMemory(device, terrain.indices.memory, nullptr);

		if (queryPool != VK_NULL_HANDLE) {
			vkDestroyQueryPool(device, queryPool, nullptr);
			vkDestroyBuffer(device, queryResult.buffer, nullptr);
			vkFreeMemory(device, queryResult.memory, nullptr);
		}
	}
}
//-----------------------------------------------------------------------------
void TerrainTessellationApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void TerrainTessellationApp::OnFrame()
{
	updateUniformBuffers();
	draw();
}
//-----------------------------------------------------------------------------
void TerrainTessellationApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{

	if (overlay->header("Settings")) {

		if (overlay->checkBox("Tessellation", &tessellation)) {
			updateUniformBuffers();
		}
		if (overlay->inputFloat("Factor", &uniformDataTessellation.tessellationFactor, 0.05f, 2)) {
			updateUniformBuffers();
		}
		if (m_physicalDevice.deviceFeatures.fillModeNonSolid) {
			if (overlay->checkBox("Wireframe", &wireframe)) {
				buildCommandBuffers();
			}
		}
	}
	if (m_physicalDevice.deviceFeatures.pipelineStatisticsQuery) {
		if (overlay->header("Pipeline statistics")) {
			overlay->text("VS invocations: %d", pipelineStats[0]);
			overlay->text("TE invocations: %d", pipelineStats[1]);
		}
	}
}
//-----------------------------------------------------------------------------
void TerrainTessellationApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void TerrainTessellationApp::OnKeyPressed(uint32_t key)
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
}
//-----------------------------------------------------------------------------
void TerrainTessellationApp::OnKeyUp(uint32_t key)
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
void TerrainTessellationApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
