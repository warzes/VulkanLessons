#include "stdafx.h"
#include "31_hdr.h"
//-----------------------------------------------------------------------------
bool HDRApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPosition(glm::vec3(0.0f, 0.0f, -6.0f));
	camera.setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 256.0f);

	loadAssets();
	prepareUniformBuffers();
	prepareoffscreenfer();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void HDRApp::OnDestroy()
{
	if (device) {
		vkDestroyPipeline(device, pipelines.skybox, nullptr);
		vkDestroyPipeline(device, pipelines.reflect, nullptr);
		vkDestroyPipeline(device, pipelines.composition, nullptr);
		vkDestroyPipeline(device, pipelines.bloom[0], nullptr);
		vkDestroyPipeline(device, pipelines.bloom[1], nullptr);
		vkDestroyPipelineLayout(device, pipelineLayouts.models, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayouts.composition, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayouts.bloomFilter, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.models, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.composition, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.bloomFilter, nullptr);
		vkDestroyRenderPass(device, offscreen.renderPass, nullptr);
		vkDestroyRenderPass(device, filterPass.renderPass, nullptr);
		vkDestroyFramebuffer(device, offscreen.frameBuffer, nullptr);
		vkDestroyFramebuffer(device, filterPass.frameBuffer, nullptr);
		vkDestroySampler(device, offscreen.sampler, nullptr);
		vkDestroySampler(device, filterPass.sampler, nullptr);
		offscreen.depth.destroy(device);
		offscreen.color[0].destroy(device);
		offscreen.color[1].destroy(device);
		filterPass.color[0].destroy(device);
		uniformBuffer.destroy();
		textures.envmap.destroy();
	}
}
//-----------------------------------------------------------------------------
void HDRApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void HDRApp::OnFrame()
{
	updateUniformBuffers();
	draw();
}
//-----------------------------------------------------------------------------
void HDRApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		if (overlay->comboBox("Object type", &models.index, modelNames)) {
			buildCommandBuffers();
		}
		overlay->inputFloat("Exposure", &uniformData.exposure, 0.025f, 3);
		if (overlay->checkBox("Bloom", &bloom)) {
			buildCommandBuffers();
		}
		if (overlay->checkBox("Skybox", &displaySkybox)) {
			buildCommandBuffers();
		}
	}
}
//-----------------------------------------------------------------------------
void HDRApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void HDRApp::OnKeyPressed(uint32_t key)
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
void HDRApp::OnKeyUp(uint32_t key)
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
void HDRApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
