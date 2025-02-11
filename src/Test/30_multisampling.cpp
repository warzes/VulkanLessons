#include "stdafx.h"
#include "30_multisampling.h"
//-----------------------------------------------------------------------------
bool MultisamplingApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 256.0f);
	camera.setRotation(glm::vec3(0.0f, -90.0f, 0.0f));
	camera.setTranslation(glm::vec3(2.5f, 2.5f, -7.5f));

	loadAssets();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void MultisamplingApp::OnDestroy()
{
	if (device) {
		vkDestroyPipeline(device, pipelines.MSAA, nullptr);
		vkDestroyPipeline(device, pipelines.MSAASampleShading, nullptr);

		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		// Destroy MSAA target
		vkDestroyImage(device, multisampleTarget.color.image, nullptr);
		vkDestroyImageView(device, multisampleTarget.color.view, nullptr);
		vkFreeMemory(device, multisampleTarget.color.memory, nullptr);
		vkDestroyImage(device, multisampleTarget.depth.image, nullptr);
		vkDestroyImageView(device, multisampleTarget.depth.view, nullptr);
		vkFreeMemory(device, multisampleTarget.depth.memory, nullptr);

		uniformBuffer.destroy();
	}
}
//-----------------------------------------------------------------------------
void MultisamplingApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void MultisamplingApp::OnFrame()
{
	updateUniformBuffers();
	draw();
}
//-----------------------------------------------------------------------------
void MultisamplingApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (m_vulkanDevice->features.sampleRateShading) {
		if (overlay->header("Settings")) {
			if (overlay->checkBox("Sample rate shading", &useSampleShading)) {
				buildCommandBuffers();
			}
		}
	}
}
//-----------------------------------------------------------------------------
void MultisamplingApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void MultisamplingApp::OnKeyPressed(uint32_t key)
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
void MultisamplingApp::OnKeyUp(uint32_t key)
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
void MultisamplingApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
