#include "stdafx.h"
#include "81_bloom.h"
//-----------------------------------------------------------------------------
bool BloomApp::OnCreate()
{
	timerSpeed *= 0.5f;
	camera.type = Camera::CameraType::lookat;
	camera.setPosition(glm::vec3(0.0f, 0.0f, -10.25f));
	camera.setRotation(glm::vec3(7.5f, -343.0f, 0.0f));
	camera.setPerspective(45.0f, (float)destWidth / (float)destHeight, 0.1f, 256.0f);

	loadAssets();
	prepareUniformBuffers();
	prepareOffscreen();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void BloomApp::OnDestroy()
{
	// Clean up used Vulkan resources
		// Note : Inherited destructor cleans up resources stored in base class

	vkDestroySampler(device, offscreenPass.sampler, nullptr);

	// Frame buffer
	for (auto& framebuffer : offscreenPass.framebuffers)
	{
		// Attachments
		vkDestroyImageView(device, framebuffer.color.view, nullptr);
		vkDestroyImage(device, framebuffer.color.image, nullptr);
		vkFreeMemory(device, framebuffer.color.mem, nullptr);
		vkDestroyImageView(device, framebuffer.depth.view, nullptr);
		vkDestroyImage(device, framebuffer.depth.image, nullptr);
		vkFreeMemory(device, framebuffer.depth.mem, nullptr);

		vkDestroyFramebuffer(device, framebuffer.framebuffer, nullptr);
	}
	vkDestroyRenderPass(device, offscreenPass.renderPass, nullptr);

	vkDestroyPipeline(device, pipelines.blurHorz, nullptr);
	vkDestroyPipeline(device, pipelines.blurVert, nullptr);
	vkDestroyPipeline(device, pipelines.phongPass, nullptr);
	vkDestroyPipeline(device, pipelines.glowPass, nullptr);
	vkDestroyPipeline(device, pipelines.skyBox, nullptr);

	vkDestroyPipelineLayout(device, pipelineLayouts.blur, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayouts.scene, nullptr);

	vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.blur, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.scene, nullptr);

	// Uniform buffers
	uniformBuffers.scene.destroy();
	uniformBuffers.skyBox.destroy();
	uniformBuffers.blurParams.destroy();

	cubemap.destroy();
}
//-----------------------------------------------------------------------------
void BloomApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void BloomApp::OnFrame()
{
	draw();
	if (!paused || camera.updated)
		updateUniformBuffersScene();
}
//-----------------------------------------------------------------------------
void BloomApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		if (overlay->checkBox("Bloom", &bloom)) {
			buildCommandBuffers();
		}
		if (overlay->inputFloat("Scale", &ubos.blurParams.blurScale, 0.1f, 2)) {
			updateUniformBuffersBlur();
		}
	}
}
//-----------------------------------------------------------------------------
void BloomApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void BloomApp::OnKeyPressed(uint32_t key)
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
void BloomApp::OnKeyUp(uint32_t key)
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
void BloomApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
