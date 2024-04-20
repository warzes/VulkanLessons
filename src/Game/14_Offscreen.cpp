#include "stdafx.h"
#include "14_Offscreen.h"
//-----------------------------------------------------------------------------
bool OffscreenApp::OnCreate()
{
	timerSpeed *= 0.25f;
	camera.type = Camera::CameraType::lookat;
	camera.setPosition(glm::vec3(0.0f, 1.0f, -6.0f));
	camera.setRotation(glm::vec3(-2.5f, 0.0f, 0.0f));
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 256.0f);

	loadAssets();
	prepareOffscreen();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void OffscreenApp::OnDestroy()
{
	if (device) {
		// Frame buffer

		// Color attachment
		vkDestroyImageView(device, offscreenPass.color.view, nullptr);
		vkDestroyImage(device, offscreenPass.color.image, nullptr);
		vkFreeMemory(device, offscreenPass.color.mem, nullptr);

		// Depth attachment
		vkDestroyImageView(device, offscreenPass.depth.view, nullptr);
		vkDestroyImage(device, offscreenPass.depth.image, nullptr);
		vkFreeMemory(device, offscreenPass.depth.mem, nullptr);

		vkDestroyRenderPass(device, offscreenPass.renderPass, nullptr);
		vkDestroySampler(device, offscreenPass.sampler, nullptr);
		vkDestroyFramebuffer(device, offscreenPass.frameBuffer, nullptr);

		vkDestroyPipeline(device, pipelines.debug, nullptr);
		vkDestroyPipeline(device, pipelines.shaded, nullptr);
		vkDestroyPipeline(device, pipelines.shadedOffscreen, nullptr);
		vkDestroyPipeline(device, pipelines.mirror, nullptr);

		vkDestroyPipelineLayout(device, pipelineLayouts.textured, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayouts.shaded, nullptr);

		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.shaded, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.textured, nullptr);

		// Uniform buffers
		uniformBuffers.vsShared.destroy();
		uniformBuffers.vsMirror.destroy();
		uniformBuffers.vsOffScreen.destroy();
	}
}
//-----------------------------------------------------------------------------
void OffscreenApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void OffscreenApp::OnFrame()
{
	draw();
	if (!paused || camera.updated)
	{
		if (!paused) {
			modelRotation.y += frameTimer * 10.0f;
		}
		updateUniformBuffers();
		updateUniformBufferOffscreen();
	}
}
//-----------------------------------------------------------------------------
void OffscreenApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		if (overlay->checkBox("Display render target", &debugDisplay)) {
			buildCommandBuffers();
		}
	}
}
//-----------------------------------------------------------------------------
void OffscreenApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void OffscreenApp::OnKeyPressed(uint32_t key)
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
void OffscreenApp::OnKeyUp(uint32_t key)
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
void OffscreenApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
