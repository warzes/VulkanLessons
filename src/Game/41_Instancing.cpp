#include "stdafx.h"
#include "41_Instancing.h"
//-----------------------------------------------------------------------------
bool InstancingApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPosition(glm::vec3(5.5f, -1.85f, -18.5f));
	camera.setRotation(glm::vec3(-17.2f, -4.7f, 0.0f));
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 1.0f, 256.0f);

	loadAssets();
	prepareInstanceData();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void InstancingApp::OnDestroy()
{
	if (device) {
		vkDestroyPipeline(device, pipelines.instancedRocks, nullptr);
		vkDestroyPipeline(device, pipelines.planet, nullptr);
		vkDestroyPipeline(device, pipelines.starfield, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		vkDestroyBuffer(device, instanceBuffer.buffer, nullptr);
		vkFreeMemory(device, instanceBuffer.memory, nullptr);
		textures.rocks.destroy();
		textures.planet.destroy();
		uniformBuffer.destroy();
	}
}
//-----------------------------------------------------------------------------
void InstancingApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void InstancingApp::OnFrame()
{
	updateUniformBuffer();
	draw();
}
//-----------------------------------------------------------------------------
void InstancingApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Statistics")) {
		overlay->text("Instances: %d", INSTANCE_COUNT);
	}
}
//-----------------------------------------------------------------------------
void InstancingApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void InstancingApp::OnKeyPressed(uint32_t key)
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
void InstancingApp::OnKeyUp(uint32_t key)
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
void InstancingApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
