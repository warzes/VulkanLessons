#include "stdafx.h"
#include "56_ssao.h"
//-----------------------------------------------------------------------------
bool SSAOApp::OnCreate()
{
	camera.type = Camera::CameraType::firstperson;
#ifndef __ANDROID__
	camera.rotationSpeed = 0.25f;
#endif
	camera.position = { 1.0f, 0.75f, 0.0f };
	camera.setRotation(glm::vec3(0.0f, 90.0f, 0.0f));
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, uboSceneParams.nearPlane, uboSceneParams.farPlane);

	loadAssets();
	prepareOffscreenFramebuffers();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void SSAOApp::OnDestroy()
{
	if (device) {
		vkDestroySampler(device, colorSampler, nullptr);

		// Attachments
		frameBuffers.offscreen.position.destroy(device);
		frameBuffers.offscreen.normal.destroy(device);
		frameBuffers.offscreen.albedo.destroy(device);
		frameBuffers.offscreen.depth.destroy(device);
		frameBuffers.ssao.color.destroy(device);
		frameBuffers.ssaoBlur.color.destroy(device);

		// Framebuffers
		frameBuffers.offscreen.destroy(device);
		frameBuffers.ssao.destroy(device);
		frameBuffers.ssaoBlur.destroy(device);

		vkDestroyPipeline(device, pipelines.offscreen, nullptr);
		vkDestroyPipeline(device, pipelines.composition, nullptr);
		vkDestroyPipeline(device, pipelines.ssao, nullptr);
		vkDestroyPipeline(device, pipelines.ssaoBlur, nullptr);

		vkDestroyPipelineLayout(device, pipelineLayouts.gBuffer, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayouts.ssao, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayouts.ssaoBlur, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayouts.composition, nullptr);

		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.gBuffer, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.ssao, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.ssaoBlur, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.composition, nullptr);

		// Uniform buffers
		uniformBuffers.sceneParams.destroy();
		uniformBuffers.ssaoKernel.destroy();
		uniformBuffers.ssaoParams.destroy();

		ssaoNoise.destroy();
	}
}
//-----------------------------------------------------------------------------
void SSAOApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void SSAOApp::OnFrame()
{
	updateUniformBufferMatrices();
	updateUniformBufferSSAOParams();
	draw();
}
//-----------------------------------------------------------------------------
void SSAOApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		overlay->checkBox("Enable SSAO", &uboSSAOParams.ssao);
		overlay->checkBox("SSAO blur", &uboSSAOParams.ssaoBlur);
		overlay->checkBox("SSAO pass only", &uboSSAOParams.ssaoOnly);
	}
}
//-----------------------------------------------------------------------------
void SSAOApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void SSAOApp::OnKeyPressed(uint32_t key)
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
void SSAOApp::OnKeyUp(uint32_t key)
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
void SSAOApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
