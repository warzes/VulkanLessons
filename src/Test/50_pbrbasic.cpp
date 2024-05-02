#include "stdafx.h"
#include "50_pbrbasic.h"
//-----------------------------------------------------------------------------
bool PBRBasicApp::OnCreate()
{
	camera.type = Camera::CameraType::firstperson;
	camera.setPosition(glm::vec3(10.0f, 13.0f, 1.8f));
	camera.setRotation(glm::vec3(-62.5f, 90.0f, 0.0f));
	camera.movementSpeed = 4.0f;
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 256.0f);
	camera.rotationSpeed = 0.25f;
	timerSpeed *= 0.25f;

	// Setup some default materials (source: https://seblagarde.wordpress.com/2011/08/17/feeding-a-physical-based-lighting-mode/)
	materials.push_back(Material("Gold", glm::vec3(1.0f, 0.765557f, 0.336057f), 0.1f, 1.0f));
	materials.push_back(Material("Copper", glm::vec3(0.955008f, 0.637427f, 0.538163f), 0.1f, 1.0f));
	materials.push_back(Material("Chromium", glm::vec3(0.549585f, 0.556114f, 0.554256f), 0.1f, 1.0f));
	materials.push_back(Material("Nickel", glm::vec3(0.659777f, 0.608679f, 0.525649f), 0.1f, 1.0f));
	materials.push_back(Material("Titanium", glm::vec3(0.541931f, 0.496791f, 0.449419f), 0.1f, 1.0f));
	materials.push_back(Material("Cobalt", glm::vec3(0.662124f, 0.654864f, 0.633732f), 0.1f, 1.0f));
	materials.push_back(Material("Platinum", glm::vec3(0.672411f, 0.637331f, 0.585456f), 0.1f, 1.0f));
	// Testing materials
	materials.push_back(Material("White", glm::vec3(1.0f), 0.1f, 1.0f));
	materials.push_back(Material("Red", glm::vec3(1.0f, 0.0f, 0.0f), 0.1f, 1.0f));
	materials.push_back(Material("Blue", glm::vec3(0.0f, 0.0f, 1.0f), 0.1f, 1.0f));
	materials.push_back(Material("Black", glm::vec3(0.0f), 0.1f, 1.0f));

	for (auto material : materials) {
		materialNames.push_back(material.name);
	}
	objectNames = { "Sphere", "Teapot", "Torusknot", "Venus" };

	materialIndex = 0;

	loadAssets();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void PBRBasicApp::OnDestroy()
{
	if (device) {
		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		uniformBuffers.object.destroy();
		uniformBuffers.params.destroy();
	}
}
//-----------------------------------------------------------------------------
void PBRBasicApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void PBRBasicApp::OnFrame()
{
	updateUniformBuffers();
	if (!paused) {
		updateLights();
	}
	draw();
}
//-----------------------------------------------------------------------------
void PBRBasicApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		if (overlay->comboBox("Material", &materialIndex, materialNames)) {
			buildCommandBuffers();
		}
		if (overlay->comboBox("Object type", &models.objectIndex, objectNames)) {
			updateUniformBuffers();
			buildCommandBuffers();
		}
	}
}
//-----------------------------------------------------------------------------
void PBRBasicApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void PBRBasicApp::OnKeyPressed(uint32_t key)
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
void PBRBasicApp::OnKeyUp(uint32_t key)
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
void PBRBasicApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
