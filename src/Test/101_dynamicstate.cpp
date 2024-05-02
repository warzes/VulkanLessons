#include "stdafx.h"
#include "101_dynamicstate.h"
//-----------------------------------------------------------------------------
bool DynamicStateApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPosition(glm::vec3(0.0f, 0.0f, -10.5f));
	camera.setRotation(glm::vec3(-25.0f, 15.0f, 0.0f));
	camera.setRotationSpeed(0.5f);
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 256.0f);

	// Dynamic states are set with vkCmd* calls in the command buffer, so we need to load the function pointers depending on extension supports
	if (hasDynamicState) {
		vkCmdSetCullModeEXT = reinterpret_cast<PFN_vkCmdSetCullModeEXT>(vkGetDeviceProcAddr(device, "vkCmdSetCullModeEXT"));
		vkCmdSetFrontFaceEXT = reinterpret_cast<PFN_vkCmdSetFrontFaceEXT>(vkGetDeviceProcAddr(device, "vkCmdSetFrontFaceEXT"));
		vkCmdSetDepthWriteEnableEXT = reinterpret_cast<PFN_vkCmdSetDepthWriteEnableEXT>(vkGetDeviceProcAddr(device, "vkCmdSetDepthWriteEnableEXT"));
		vkCmdSetDepthTestEnableEXT = reinterpret_cast<PFN_vkCmdSetDepthTestEnable>(vkGetDeviceProcAddr(device, "vkCmdSetDepthTestEnableEXT"));
	}

	if (hasDynamicState2) {
		vkCmdSetRasterizerDiscardEnableEXT = reinterpret_cast<PFN_vkCmdSetRasterizerDiscardEnableEXT>(vkGetDeviceProcAddr(device, "vkCmdSetRasterizerDiscardEnableEXT"));
	}

	if (hasDynamicState3) {
		vkCmdSetColorBlendEnableEXT = reinterpret_cast<PFN_vkCmdSetColorBlendEnableEXT>(vkGetDeviceProcAddr(device, "vkCmdSetColorBlendEnableEXT"));
		vkCmdSetColorBlendEquationEXT = reinterpret_cast<PFN_vkCmdSetColorBlendEquationEXT>(vkGetDeviceProcAddr(device, "vkCmdSetColorBlendEquationEXT"));
	}

	loadAssets();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void DynamicStateApp::OnDestroy()
{
	if (device) {
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		uniformBuffer.destroy();
	}
}
//-----------------------------------------------------------------------------
void DynamicStateApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void DynamicStateApp::OnFrame()
{
	updateUniformBuffers();
	draw();
}
//-----------------------------------------------------------------------------
void DynamicStateApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	bool rebuildCB = false;
	if (overlay->header("Dynamic state")) {
		if (hasDynamicState) {
			rebuildCB = overlay->comboBox("Cull mode", &dynamicState.cullMode, { "none", "front", "back" });
			rebuildCB |= overlay->comboBox("Front face", &dynamicState.frontFace, { "Counter clockwise", "Clockwise" });
			rebuildCB |= overlay->checkBox("Depth test", &dynamicState.depthTest);
			rebuildCB |= overlay->checkBox("Depth write", &dynamicState.depthWrite);
		}
		else {
			overlay->text("Extension not supported");
		}
	}
	if (overlay->header("Dynamic state 2")) {
		if (hasDynamicState2) {
			rebuildCB |= overlay->checkBox("Rasterizer discard", &dynamicState2.rasterizerDiscardEnable);
		}
		else {
			overlay->text("Extension not supported");
		}
	}
	if (overlay->header("Dynamic state 3")) {
		if (hasDynamicState3) {
			rebuildCB |= overlay->checkBox("Color blend", &dynamicState3.colorBlendEnable);
			rebuildCB |= overlay->colorPicker("Clear color", clearColor);
		}
		else {
			overlay->text("Extension not supported");
		}
	}
	if (rebuildCB) {
		buildCommandBuffers();
	}
}
//-----------------------------------------------------------------------------
void DynamicStateApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void DynamicStateApp::OnKeyPressed(uint32_t key)
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
void DynamicStateApp::OnKeyUp(uint32_t key)
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
void DynamicStateApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
