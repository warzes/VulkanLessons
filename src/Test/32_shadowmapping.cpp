#include "stdafx.h"
#include "32_shadowmapping.h"
//-----------------------------------------------------------------------------
bool ShadowMappingApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPosition(glm::vec3(0.0f, 0.0f, -12.5f));
	camera.setRotation(glm::vec3(-25.0f, -390.0f, 0.0f));
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 1.0f, 256.0f);
	timerSpeed *= 0.5f;

	loadAssets();
	prepareOffscreenFramebuffer();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void ShadowMappingApp::OnDestroy()
{
	if (device) {
		// Frame buffer
		vkDestroySampler(device, offscreenPass.depthSampler, nullptr);

		// Depth attachment
		vkDestroyImageView(device, offscreenPass.depth.view, nullptr);
		vkDestroyImage(device, offscreenPass.depth.image, nullptr);
		vkFreeMemory(device, offscreenPass.depth.mem, nullptr);

		vkDestroyFramebuffer(device, offscreenPass.frameBuffer, nullptr);

		vkDestroyRenderPass(device, offscreenPass.renderPass, nullptr);

		vkDestroyPipeline(device, pipelines.debug, nullptr);
		vkDestroyPipeline(device, pipelines.offscreen, nullptr);
		vkDestroyPipeline(device, pipelines.sceneShadow, nullptr);
		vkDestroyPipeline(device, pipelines.sceneShadowPCF, nullptr);

		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		// Uniform buffers
		uniformBuffers.offscreen.destroy();
		uniformBuffers.scene.destroy();
	}
}
//-----------------------------------------------------------------------------
void ShadowMappingApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void ShadowMappingApp::OnFrame()
{
	if (!paused || camera.updated) {
		updateLight();
		updateUniformBufferOffscreen();
		updateUniformBuffers();
	}
	draw();
}
//-----------------------------------------------------------------------------
void ShadowMappingApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		if (overlay->comboBox("Scenes", &sceneIndex, sceneNames)) {
			buildCommandBuffers();
		}
		if (overlay->checkBox("Display shadow render target", &displayShadowMap)) {
			buildCommandBuffers();
		}
		if (overlay->checkBox("PCF filtering", &filterPCF)) {
			buildCommandBuffers();
		}
	}
}
//-----------------------------------------------------------------------------
void ShadowMappingApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void ShadowMappingApp::OnKeyPressed(uint32_t key)
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
void ShadowMappingApp::OnKeyUp(uint32_t key)
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
void ShadowMappingApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
