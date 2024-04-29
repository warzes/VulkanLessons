#include "stdafx.h"
#include "71_distancefieldfonts.h"
//-----------------------------------------------------------------------------
bool DistanceFieldFontsApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPosition(glm::vec3(0.0f, 0.0f, -2.0f));
	camera.setRotation(glm::vec3(0.0f));
	camera.setPerspective(splitScreen ? 30.0f : 45.0f, (float)destWidth / (float)(destHeight * ((splitScreen) ? 0.5f : 1.0f)), 1.0f, 256.0f);

	parsebmFont();
	loadAssets();
	generateText("Vulkan");
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void DistanceFieldFontsApp::OnDestroy()
{
	if (device) {
		textures.fontSDF.destroy();
		textures.fontBitmap.destroy();
		vkDestroyPipeline(device, pipelines.sdf, nullptr);
		vkDestroyPipeline(device, pipelines.bitmap, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		vertexBuffer.destroy();
		indexBuffer.destroy();
		uniformBuffer.destroy();
	}
}
//-----------------------------------------------------------------------------
void DistanceFieldFontsApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void DistanceFieldFontsApp::OnFrame()
{
	updateUniformBuffers();
	draw();
}
//-----------------------------------------------------------------------------
void DistanceFieldFontsApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		bool outline = (uniformData.outline == 1.0f);
		if (overlay->checkBox("Outline", &outline)) {
			uniformData.outline = outline ? 1.0f : 0.0f;
		}
		if (overlay->checkBox("Splitscreen", &splitScreen)) {
			camera.setPerspective(splitScreen ? 30.0f : 45.0f, (float)destWidth / (float)(destHeight * ((splitScreen) ? 0.5f : 1.0f)), 1.0f, 256.0f);
			buildCommandBuffers();
		}
	}
}
//-----------------------------------------------------------------------------
void DistanceFieldFontsApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void DistanceFieldFontsApp::OnKeyPressed(uint32_t key)
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
void DistanceFieldFontsApp::OnKeyUp(uint32_t key)
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
void DistanceFieldFontsApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
