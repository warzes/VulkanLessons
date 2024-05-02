#include "stdafx.h"
#include "42_indirectdraw.h"
//-----------------------------------------------------------------------------
bool IndirectDrawApp::OnCreate()
{
	camera.type = Camera::CameraType::firstperson;
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 512.0f);
	camera.setRotation(glm::vec3(-12.0f, 159.0f, 0.0f));
	camera.setTranslation(glm::vec3(0.4f, 1.25f, 0.0f));
	camera.movementSpeed = 5.0f;

	loadAssets();
	prepareIndirectData();
	prepareInstanceData();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void IndirectDrawApp::OnDestroy()
{
	if (device) {
		vkDestroyPipeline(device, pipelines.plants, nullptr);
		vkDestroyPipeline(device, pipelines.ground, nullptr);
		vkDestroyPipeline(device, pipelines.skysphere, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		textures.plants.destroy();
		textures.ground.destroy();
		instanceBuffer.destroy();
		indirectCommandsBuffer.destroy();
		uniformBuffer.destroy();
	}
}
//-----------------------------------------------------------------------------
void IndirectDrawApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void IndirectDrawApp::OnFrame()
{
	updateUniformBuffer();
	draw();
}
//-----------------------------------------------------------------------------
void IndirectDrawApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (!m_vulkanDevice->features.multiDrawIndirect) {
		if (overlay->header("Info")) {
			overlay->text("multiDrawIndirect not supported");
		}
	}
	if (overlay->header("Statistics")) {
		overlay->text("Objects: %d", objectCount);
	}
}
//-----------------------------------------------------------------------------
void IndirectDrawApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void IndirectDrawApp::OnKeyPressed(uint32_t key)
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
void IndirectDrawApp::OnKeyUp(uint32_t key)
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
void IndirectDrawApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
