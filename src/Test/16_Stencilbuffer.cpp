#include "stdafx.h"
#include "16_Stencilbuffer.h"
//-----------------------------------------------------------------------------
EngineCreateInfo StencilbufferApp::GetCreateInfo() const
{
	EngineCreateInfo ci{};
	// This samples requires a format that supports depth AND stencil, which will be handled by the base class
	ci.render.requiresStencil = true;
	return ci;
}
//-----------------------------------------------------------------------------
bool StencilbufferApp::OnCreate()
{
	timerSpeed *= 0.25f;
	camera.type = Camera::CameraType::lookat;
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 512.0f);
	camera.setRotation(glm::vec3(2.5f, -35.0f, 0.0f));
	camera.setTranslation(glm::vec3(0.0f, 0.0f, -2.0f));

	loadAssets();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void StencilbufferApp::OnDestroy()
{
	if (device) {
		vkDestroyPipeline(device, pipelines.stencil, nullptr);
		vkDestroyPipeline(device, pipelines.outline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		uniformBuffer.destroy();
	}
}
//-----------------------------------------------------------------------------
void StencilbufferApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void StencilbufferApp::OnFrame()
{
	updateUniformBuffers();
	draw();
}
//-----------------------------------------------------------------------------
void StencilbufferApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		if (overlay->inputFloat("Outline width", &uniformData.outlineWidth, 0.01f, 2)) {
			updateUniformBuffers();
		}
	}
}
//-----------------------------------------------------------------------------
void StencilbufferApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void StencilbufferApp::OnKeyPressed(uint32_t key)
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
void StencilbufferApp::OnKeyUp(uint32_t key)
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
void StencilbufferApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
