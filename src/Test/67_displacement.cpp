#include "stdafx.h"
#include "67_displacement.h"
//-----------------------------------------------------------------------------
bool DisplacementApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPosition(glm::vec3(0.0f, 0.0f, -1.25f));
	camera.setRotation(glm::vec3(-20.0f, 45.0f, 0.0f));
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 256.0f);

	loadAssets();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void DisplacementApp::OnDestroy()
{
	if (device) {
		vkDestroyPipeline(device, pipelines.solid, nullptr);
		if (pipelines.wireframe != VK_NULL_HANDLE) {
			vkDestroyPipeline(device, pipelines.wireframe, nullptr);
		};
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		uniformBuffer.destroy();
		colorHeightMap.destroy();
	}
}
//-----------------------------------------------------------------------------
void DisplacementApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void DisplacementApp::OnFrame()
{
	updateUniformBuffers();
	draw();
}
//-----------------------------------------------------------------------------
void DisplacementApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		if (overlay->checkBox("Tessellation displacement", &displacement)) {
			updateUniformBuffers();
		}
		if (overlay->inputFloat("Strength", &uniformData.tessStrength, 0.025f, 3)) {
			updateUniformBuffers();
		}
		if (overlay->inputFloat("Level", &uniformData.tessLevel, 0.5f, 2)) {
			updateUniformBuffers();
		}
		if (m_adapter.deviceFeatures.fillModeNonSolid) {
			if (overlay->checkBox("Splitscreen", &splitScreen)) {
				buildCommandBuffers();
				updateUniformBuffers();
			}
		}

	}
}
//-----------------------------------------------------------------------------
void DisplacementApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void DisplacementApp::OnKeyPressed(uint32_t key)
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
void DisplacementApp::OnKeyUp(uint32_t key)
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
void DisplacementApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
