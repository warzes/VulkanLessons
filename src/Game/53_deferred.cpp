#include "stdafx.h"
#include "53_deferred.h"
//-----------------------------------------------------------------------------
bool DefferedApp::OnCreate()
{
	camera.type = Camera::CameraType::firstperson;
	camera.movementSpeed = 5.0f;
#ifndef __ANDROID__
	camera.rotationSpeed = 0.25f;
#endif
	camera.position = { 2.15f, 0.3f, -8.75f };
	camera.setRotation(glm::vec3(-0.75f, 12.5f, 0.0f));
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 256.0f);

	loadAssets();
	prepareOffscreenFramebuffer();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();
	buildDeferredCommandBuffer();

	return true;
}
//-----------------------------------------------------------------------------
void DefferedApp::OnDestroy()
{
	if (device) {
		vkDestroySampler(device, colorSampler, nullptr);

		// Frame buffer

		// Color attachments
		vkDestroyImageView(device, offScreenFrameBuf.position.view, nullptr);
		vkDestroyImage(device, offScreenFrameBuf.position.image, nullptr);
		vkFreeMemory(device, offScreenFrameBuf.position.mem, nullptr);

		vkDestroyImageView(device, offScreenFrameBuf.normal.view, nullptr);
		vkDestroyImage(device, offScreenFrameBuf.normal.image, nullptr);
		vkFreeMemory(device, offScreenFrameBuf.normal.mem, nullptr);

		vkDestroyImageView(device, offScreenFrameBuf.albedo.view, nullptr);
		vkDestroyImage(device, offScreenFrameBuf.albedo.image, nullptr);
		vkFreeMemory(device, offScreenFrameBuf.albedo.mem, nullptr);

		// Depth attachment
		vkDestroyImageView(device, offScreenFrameBuf.depth.view, nullptr);
		vkDestroyImage(device, offScreenFrameBuf.depth.image, nullptr);
		vkFreeMemory(device, offScreenFrameBuf.depth.mem, nullptr);

		vkDestroyFramebuffer(device, offScreenFrameBuf.frameBuffer, nullptr);

		vkDestroyPipeline(device, pipelines.composition, nullptr);
		vkDestroyPipeline(device, pipelines.offscreen, nullptr);

		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		// Uniform buffers
		uniformBuffers.offscreen.destroy();
		uniformBuffers.composition.destroy();

		vkDestroyRenderPass(device, offScreenFrameBuf.renderPass, nullptr);

		textures.model.colorMap.destroy();
		textures.model.normalMap.destroy();
		textures.floor.colorMap.destroy();
		textures.floor.normalMap.destroy();

		vkDestroySemaphore(device, offscreenSemaphore, nullptr);
	}
}
//-----------------------------------------------------------------------------
void DefferedApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void DefferedApp::OnFrame()
{
	updateUniformBufferComposition();
	updateUniformBufferOffscreen();
	draw();
}
//-----------------------------------------------------------------------------
void DefferedApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		overlay->comboBox("Display", &debugDisplayTarget, { "Final composition", "Position", "Normals", "Albedo", "Specular" });
	}
}
//-----------------------------------------------------------------------------
void DefferedApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void DefferedApp::OnKeyPressed(uint32_t key)
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
void DefferedApp::OnKeyUp(uint32_t key)
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
void DefferedApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
