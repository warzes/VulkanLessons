#include "stdafx.h"
#include "34_shadowmappingomni.h"
//-----------------------------------------------------------------------------
bool ShadowMappingOmniApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPerspective(45.0f, (float)destWidth / (float)destHeight, zNear, zFar);
	camera.setRotation(glm::vec3(-20.5f, -673.0f, 0.0f));
	camera.setPosition(glm::vec3(0.0f, 0.5f, -15.0f));
	timerSpeed *= 0.5f;

	loadAssets();
	prepareUniformBuffers();
	prepareCubeMap();
	setupDescriptors();
	prepareOffscreenRenderpass();
	preparePipelines();
	prepareOffscreenFramebuffer();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void ShadowMappingOmniApp::OnDestroy()
{
	if (device) {
		// Cube map
		for (uint32_t i = 0; i < 6; i++) {
			vkDestroyImageView(device, shadowCubeMapFaceImageViews[i], nullptr);
		}

		vkDestroyImageView(device, shadowCubeMap.view, nullptr);
		vkDestroyImage(device, shadowCubeMap.image, nullptr);
		vkDestroySampler(device, shadowCubeMap.sampler, nullptr);
		vkFreeMemory(device, shadowCubeMap.deviceMemory, nullptr);

		// Depth attachment
		vkDestroyImageView(device, offscreenPass.depth.view, nullptr);
		vkDestroyImage(device, offscreenPass.depth.image, nullptr);
		vkFreeMemory(device, offscreenPass.depth.mem, nullptr);

		for (uint32_t i = 0; i < 6; i++)
		{
			vkDestroyFramebuffer(device, offscreenPass.frameBuffers[i], nullptr);
		}

		vkDestroyRenderPass(device, offscreenPass.renderPass, nullptr);

		// Pipelines
		vkDestroyPipeline(device, pipelines.scene, nullptr);
		vkDestroyPipeline(device, pipelines.offscreen, nullptr);
		vkDestroyPipeline(device, pipelines.cubemapDisplay, nullptr);

		vkDestroyPipelineLayout(device, pipelineLayouts.scene, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayouts.offscreen, nullptr);

		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		// Uniform buffers
		uniformBuffers.offscreen.destroy();
		uniformBuffers.scene.destroy();
	}
}
//-----------------------------------------------------------------------------
void ShadowMappingOmniApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void ShadowMappingOmniApp::OnFrame()
{
	updateUniformBuffers();
	updateUniformBufferOffscreen();
	draw();
}
//-----------------------------------------------------------------------------
void ShadowMappingOmniApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		if (overlay->checkBox("Display shadow cube render target", &displayCubeMap)) {
			buildCommandBuffers();
		}
	}
}
//-----------------------------------------------------------------------------
void ShadowMappingOmniApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void ShadowMappingOmniApp::OnKeyPressed(uint32_t key)
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
void ShadowMappingOmniApp::OnKeyUp(uint32_t key)
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
void ShadowMappingOmniApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
