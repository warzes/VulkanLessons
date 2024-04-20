#include "stdafx.h"
#include "13_Subpasses.h"
//-----------------------------------------------------------------------------
EngineCreateInfo SubpassesApp::GetCreateInfo() const
{
	EngineCreateInfo ci{};
	ci.render.overlay.subpass = 2;
	return ci;
}
//-----------------------------------------------------------------------------
bool SubpassesApp::OnCreate()
{
	camera.type = Camera::CameraType::firstperson;
	camera.movementSpeed = 5.0f;
	camera.rotationSpeed = 0.25f;
	camera.setPosition(glm::vec3(-3.2f, 1.0f, 5.9f));
	camera.setRotation(glm::vec3(0.5f, 210.05f, 0.0f));
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 256.0f);

	loadAssets();
	prepareUniformBuffers();
	initLights();
	setupDescriptors();
	preparePipelines();
	prepareCompositionPass();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void SubpassesApp::OnDestroy()
{
	// Clean up used Vulkan resources
		// Note : Inherited destructor cleans up resources stored in base class
	vkDestroyPipeline(device, pipelines.offscreen, nullptr);
	vkDestroyPipeline(device, pipelines.composition, nullptr);
	vkDestroyPipeline(device, pipelines.transparent, nullptr);

	vkDestroyPipelineLayout(device, pipelineLayouts.offscreen, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayouts.composition, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayouts.transparent, nullptr);

	vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.scene, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.composition, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.transparent, nullptr);

	clearAttachment(&attachments.position);
	clearAttachment(&attachments.normal);
	clearAttachment(&attachments.albedo);

	textures.glass.destroy();
	buffers.GBuffer.destroy();
	buffers.lights.destroy();
}
//-----------------------------------------------------------------------------
void SubpassesApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void SubpassesApp::OnFrame()
{
	if (camera.updated) {
		updateUniformBufferDeferredMatrices();
	}
	draw();
}
//-----------------------------------------------------------------------------
void SubpassesApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Subpasses")) {
		overlay->text("0: Deferred G-Buffer creation");
		overlay->text("1: Deferred composition");
		overlay->text("2: Forward transparency");
	}
	if (overlay->header("Settings")) {
		if (overlay->button("Randomize lights")) {
			initLights();
		}
	}
}
//-----------------------------------------------------------------------------
void SubpassesApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void SubpassesApp::OnKeyPressed(uint32_t key)
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
void SubpassesApp::OnKeyUp(uint32_t key)
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
void SubpassesApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
