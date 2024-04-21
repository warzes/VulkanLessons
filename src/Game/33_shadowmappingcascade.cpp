#include "stdafx.h"
#include "33_shadowmappingcascade.h"
//-----------------------------------------------------------------------------
bool ShadowMappingCascadeApp::OnCreate()
{
	timerSpeed *= 0.025f;
	camera.type = Camera::CameraType::firstperson;
	camera.movementSpeed = 2.5f;
	camera.setPerspective(45.0f, (float)destWidth / (float)destHeight, zNear, zFar);
	camera.setPosition(glm::vec3(-0.12f, 1.14f, -2.25f));
	camera.setRotation(glm::vec3(-17.0f, 7.0f, 0.0f));
	timer = 0.2f;

	loadAssets();
	updateLight();
	updateCascades();
	prepareDepthPass();
	prepareUniformBuffers();
	setupLayoutsAndDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void ShadowMappingCascadeApp::OnDestroy()
{
	for (auto cascade : cascades) {
		cascade.destroy(device);
	}
	depth.destroy(device);

	vkDestroyRenderPass(device, depthPass.renderPass, nullptr);

	vkDestroyPipeline(device, pipelines.debugShadowMap, nullptr);
	vkDestroyPipeline(device, depthPass.pipeline, nullptr);
	vkDestroyPipeline(device, pipelines.sceneShadow, nullptr);
	vkDestroyPipeline(device, pipelines.sceneShadowPCF, nullptr);

	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyPipelineLayout(device, depthPass.pipelineLayout, nullptr);

	vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.base, nullptr);

	depthPass.uniformBuffer.destroy();
	uniformBuffers.VS.destroy();
	uniformBuffers.FS.destroy();
}
//-----------------------------------------------------------------------------
void ShadowMappingCascadeApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void ShadowMappingCascadeApp::OnFrame()
{
	draw();
	if (!paused || camera.updated) {
		updateLight();
		updateCascades();
		updateUniformBuffers();
	}
}
//-----------------------------------------------------------------------------
void ShadowMappingCascadeApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		if (overlay->sliderFloat("Split lambda", &cascadeSplitLambda, 0.1f, 1.0f)) {
			updateCascades();
			updateUniformBuffers();
		}
		if (overlay->checkBox("Color cascades", &colorCascades)) {
			updateUniformBuffers();
		}
		if (overlay->checkBox("Display depth map", &displayDepthMap)) {
			buildCommandBuffers();
		}
		if (displayDepthMap) {
			if (overlay->sliderInt("Cascade", &displayDepthMapCascadeIndex, 0, SHADOW_MAP_CASCADE_COUNT - 1)) {
				buildCommandBuffers();
			}
		}
		if (overlay->checkBox("PCF filtering", &filterPCF)) {
			buildCommandBuffers();
		}
	}
}
//-----------------------------------------------------------------------------
void ShadowMappingCascadeApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void ShadowMappingCascadeApp::OnKeyPressed(uint32_t key)
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
void ShadowMappingCascadeApp::OnKeyUp(uint32_t key)
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
void ShadowMappingCascadeApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
