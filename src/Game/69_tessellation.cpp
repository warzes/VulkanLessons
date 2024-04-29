#include "stdafx.h"
#include "69_tessellation.h"
//-----------------------------------------------------------------------------
bool TessellationApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPosition(glm::vec3(0.0f, 0.0f, -4.0f));
	camera.setRotation(glm::vec3(-350.0f, 60.0f, 0.0f));
	camera.setPerspective(45.0f, (float)(destWidth * ((splitScreen) ? 0.5f : 1.0f)) / (float)destHeight, 0.1f, 256.0f);

	loadAssets();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void TessellationApp::OnDestroy()
{
	if (device) {
		// Clean up used Vulkan resources
		// Note : Inherited destructor cleans up resources stored in base class
		vkDestroyPipeline(device, pipelines.solid, nullptr);
		if (pipelines.wire != VK_NULL_HANDLE) {
			vkDestroyPipeline(device, pipelines.wire, nullptr);
		};
		vkDestroyPipeline(device, pipelines.solidPassThrough, nullptr);
		if (pipelines.wirePassThrough != VK_NULL_HANDLE) {
			vkDestroyPipeline(device, pipelines.wirePassThrough, nullptr);
		};

		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		uniformBuffer.destroy();
	}
}
//-----------------------------------------------------------------------------
void TessellationApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void TessellationApp::OnFrame()
{
	updateUniformBuffers();
	draw();
}
//-----------------------------------------------------------------------------
void TessellationApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		if (overlay->inputFloat("Tessellation level", &uniformData.tessLevel, 0.25f, 2)) {
			updateUniformBuffers();
		}
		if (deviceFeatures.fillModeNonSolid) {
			if (overlay->checkBox("Wireframe", &wireframe)) {
				updateUniformBuffers();
				buildCommandBuffers();
			}
			if (overlay->checkBox("Splitscreen", &splitScreen)) {
				camera.setPerspective(45.0f, (float)(destWidth * ((splitScreen) ? 0.5f : 1.0f)) / (float)destHeight, 0.1f, 256.0f);
				updateUniformBuffers();
				buildCommandBuffers();
			}
		}
	}
}
//-----------------------------------------------------------------------------
void TessellationApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void TessellationApp::OnKeyPressed(uint32_t key)
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
void TessellationApp::OnKeyUp(uint32_t key)
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
void TessellationApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
