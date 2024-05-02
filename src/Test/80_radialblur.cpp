#include "stdafx.h"
#include "80_radialblur.h"
//-----------------------------------------------------------------------------
bool RadialBlurApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPosition(glm::vec3(0.0f, 0.0f, -17.5f));
	camera.setRotation(glm::vec3(-16.25f, -28.75f, 0.0f));
	camera.setPerspective(45.0f, (float)destWidth / (float)destHeight, 1.0f, 256.0f);
	timerSpeed *= 0.5f;

	loadAssets();
	prepareOffscreen();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void RadialBlurApp::OnDestroy()
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

		vkDestroyPipeline(device, pipelines.radialBlur, nullptr);
		vkDestroyPipeline(device, pipelines.phongPass, nullptr);
		vkDestroyPipeline(device, pipelines.colorPass, nullptr);
		vkDestroyPipeline(device, pipelines.offscreenDisplay, nullptr);

		vkDestroyPipelineLayout(device, pipelineLayouts.radialBlur, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayouts.scene, nullptr);

		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.scene, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.radialBlur, nullptr);

		uniformBuffers.scene.destroy();
		uniformBuffers.blurParams.destroy();

		gradientTexture.destroy();
	}
}
//-----------------------------------------------------------------------------
void RadialBlurApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void RadialBlurApp::OnFrame()
{
	updateUniformBuffers();
	draw();
}
//-----------------------------------------------------------------------------
void RadialBlurApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		if (overlay->checkBox("Radial blur", &blur)) {
			buildCommandBuffers();
		}
		if (overlay->checkBox("Display render target only", &displayTexture)) {
			buildCommandBuffers();
		}
		if (blur) {
			if (overlay->header("Blur parameters")) {
				bool updateParams = false;
				updateParams |= overlay->sliderFloat("Scale", &uniformDataBlurParams.radialBlurScale, 0.1f, 1.0f);
				updateParams |= overlay->sliderFloat("Strength", &uniformDataBlurParams.radialBlurStrength, 0.1f, 2.0f);
				updateParams |= overlay->sliderFloat("Horiz. origin", &uniformDataBlurParams.radialOrigin.x, 0.0f, 1.0f);
				updateParams |= overlay->sliderFloat("Vert. origin", &uniformDataBlurParams.radialOrigin.y, 0.0f, 1.0f);
				if (updateParams) {
					updateUniformBuffersBlurParams();
				}
			}
		}
	}
}
//-----------------------------------------------------------------------------
void RadialBlurApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void RadialBlurApp::OnKeyPressed(uint32_t key)
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
void RadialBlurApp::OnKeyUp(uint32_t key)
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
void RadialBlurApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
