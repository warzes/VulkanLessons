#include "stdafx.h"
#include "54_deferredmultisampling.h"
//-----------------------------------------------------------------------------
bool DeferredMultisamplingApp::OnCreate()
{
	camera.type = Camera::CameraType::firstperson;
	camera.movementSpeed = 5.0f;
#ifndef __ANDROID__
	camera.rotationSpeed = 0.25f;
#endif
	camera.position = { 2.15f, 0.3f, -8.75f };
	camera.setRotation(glm::vec3(-0.75f, 12.5f, 0.0f));
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 256.0f);

	sampleCount = getMaxUsableSampleCount();
	loadAssets();
	deferredSetup();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();
	buildDeferredCommandBuffer();

	return true;
}
//-----------------------------------------------------------------------------
void DeferredMultisamplingApp::OnDestroy()
{
	if (device) {
		// Frame buffers
		if (offscreenframeBuffers) {
			delete offscreenframeBuffers;
		}

		vkDestroyPipeline(device, pipelines.deferred, nullptr);
		vkDestroyPipeline(device, pipelines.deferredNoMSAA, nullptr);
		vkDestroyPipeline(device, pipelines.offscreen, nullptr);
		vkDestroyPipeline(device, pipelines.offscreenSampleShading, nullptr);

		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		// Uniform buffers
		uniformBuffers.offscreen.destroy();
		uniformBuffers.composition.destroy();

		textures.model.colorMap.destroy();
		textures.model.normalMap.destroy();
		textures.background.colorMap.destroy();
		textures.background.normalMap.destroy();

		vkDestroySemaphore(device, offscreenSemaphore, nullptr);
	}
}
//-----------------------------------------------------------------------------
void DeferredMultisamplingApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void DeferredMultisamplingApp::OnFrame()
{
	updateUniformBufferOffscreen();
	draw();
}
//-----------------------------------------------------------------------------
void DeferredMultisamplingApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		if (overlay->comboBox("Display", &debugDisplayTarget, { "Final composition", "Position", "Normals", "Albedo", "Specular" })) {
			updateUniformBufferDeferred();
		}
		if (overlay->checkBox("MSAA", &useMSAA)) {
			buildCommandBuffers();
		}
		if (m_vulkanDevice->features.sampleRateShading) {
			if (overlay->checkBox("Sample rate shading", &useSampleShading)) {
				buildDeferredCommandBuffer();
			}
		}
	}
}
//-----------------------------------------------------------------------------
void DeferredMultisamplingApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void DeferredMultisamplingApp::OnKeyPressed(uint32_t key)
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
void DeferredMultisamplingApp::OnKeyUp(uint32_t key)
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
void DeferredMultisamplingApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
