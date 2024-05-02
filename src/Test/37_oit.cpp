#include "stdafx.h"
#include "37_oit.h"
//-----------------------------------------------------------------------------
bool OITApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPosition(glm::vec3(0.0f, 0.0f, -6.0f));
	camera.setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 256.0f);

	loadAssets();
	prepareUniformBuffers();
	prepareGeometryPass();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();
	updateUniformBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void OITApp::OnDestroy()
{
	if (device) {
		vkDestroyPipeline(device, pipelines.geometry, nullptr);
		vkDestroyPipeline(device, pipelines.color, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayouts.geometry, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayouts.color, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.geometry, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.color, nullptr);
		destroyGeometryPass();
		renderPassUniformBuffer.destroy();
	}
}
//-----------------------------------------------------------------------------
void OITApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void OITApp::OnFrame()
{
	updateUniformBuffers();
	draw();
}
//-----------------------------------------------------------------------------
void OITApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{

}
//-----------------------------------------------------------------------------
void OITApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);

	destroyGeometryPass();
	prepareGeometryPass();
	vkResetDescriptorPool(device, descriptorPool, 0);
	setupDescriptors();
	resized = false;
	buildCommandBuffers();
}
//-----------------------------------------------------------------------------
void OITApp::OnKeyPressed(uint32_t key)
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
void OITApp::OnKeyUp(uint32_t key)
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
void OITApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
