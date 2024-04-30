#include "stdafx.h"
#include "44_pipelinestatistics.h"
//-----------------------------------------------------------------------------
bool PipelineStatisticsApp::OnCreate()
{
	camera.type = Camera::CameraType::firstperson;
	camera.setPosition(glm::vec3(-3.0f, 1.0f, -2.75f));
	camera.setRotation(glm::vec3(-15.25f, -46.5f, 0.0f));
	camera.movementSpeed = 4.0f;
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 256.0f);
	camera.rotationSpeed = 0.25f;

	loadAssets();
	setupQueryPool();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void PipelineStatisticsApp::OnDestroy()
{
	if (device) {
		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		vkDestroyQueryPool(device, queryPool, nullptr);
		uniformBuffer.destroy();
	}
}
//-----------------------------------------------------------------------------
void PipelineStatisticsApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void PipelineStatisticsApp::OnFrame()
{
	updateUniformBuffers();
	draw();
}
//-----------------------------------------------------------------------------
void PipelineStatisticsApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		if (overlay->comboBox("Object type", &models.objectIndex, models.names)) {
			updateUniformBuffers();
			buildCommandBuffers();
		}
		if (overlay->sliderInt("Grid size", &gridSize, 1, 10)) {
			buildCommandBuffers();
		}
		// To avoid having to create pipelines for all the settings up front, we recreate a single pipelin with different settings instead
		bool recreatePipeline{ false };
		std::vector<std::string> cullModeNames = { "None", "Front", "Back", "Back and front" };
		recreatePipeline |= overlay->comboBox("Cull mode", &cullMode, cullModeNames);
		recreatePipeline |= overlay->checkBox("Blending", &blending);
		recreatePipeline |= overlay->checkBox("Discard", &discard);
		// These features may not be supported by all implementations
		if (m_physicalDevice.deviceFeatures.fillModeNonSolid) {
			recreatePipeline |= overlay->checkBox("Wireframe", &wireframe);
		}
		if (m_physicalDevice.deviceFeatures.tessellationShader) {
			recreatePipeline |= overlay->checkBox("Tessellation", &tessellation);
		}
		if (recreatePipeline) {
			preparePipelines();
			buildCommandBuffers();
		}
	}
	if (!pipelineStats.empty()) {
		if (overlay->header("Pipeline statistics")) {
			for (auto i = 0; i < pipelineStats.size(); i++) {
				std::string caption = pipelineStatNames[i] + ": %d";
				overlay->text(caption.c_str(), pipelineStats[i]);
			}
		}
	}
}
//-----------------------------------------------------------------------------
void PipelineStatisticsApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void PipelineStatisticsApp::OnKeyPressed(uint32_t key)
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
void PipelineStatisticsApp::OnKeyUp(uint32_t key)
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
void PipelineStatisticsApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
