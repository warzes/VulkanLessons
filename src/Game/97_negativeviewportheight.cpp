#include "stdafx.h"
#include "97_negativeviewportheight.h"
//-----------------------------------------------------------------------------
bool NegativeViewportHeightApp::OnCreate()
{
	loadAssets();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void NegativeViewportHeightApp::OnDestroy()
{
	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	textures.CW.destroy();
	textures.CCW.destroy();
	quad.destroy();
}
//-----------------------------------------------------------------------------
void NegativeViewportHeightApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void NegativeViewportHeightApp::OnFrame()
{
	draw();
}
//-----------------------------------------------------------------------------
void NegativeViewportHeightApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Scene")) {
		overlay->text("Quad type");
		if (overlay->comboBox("##quadtype", &quadType, { "VK (y negative)", "GL (y positive)" })) {
			buildCommandBuffers();
		}
	}

	if (overlay->header("Viewport")) {
		if (overlay->checkBox("Negative viewport height", &negativeViewport)) {
			buildCommandBuffers();
		}
		if (overlay->sliderFloat("offset x", &offsetx, -(float)destWidth, (float)destWidth)) {
			buildCommandBuffers();
		}
		if (overlay->sliderFloat("offset y", &offsety, -(float)destHeight, (float)destHeight)) {
			buildCommandBuffers();
		}
	}
	if (overlay->header("Pipeline")) {
		overlay->text("Winding order");
		if (overlay->comboBox("##windingorder", &windingOrder, { "clock wise", "counter clock wise" })) {
			preparePipelines();
		}
		overlay->text("Cull mode");
		if (overlay->comboBox("##cullmode", &cullMode, { "none", "front face", "back face" })) {
			preparePipelines();
		}
	}
}
//-----------------------------------------------------------------------------
void NegativeViewportHeightApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void NegativeViewportHeightApp::OnKeyPressed(uint32_t key)
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
void NegativeViewportHeightApp::OnKeyUp(uint32_t key)
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
void NegativeViewportHeightApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
