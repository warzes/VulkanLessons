#include "stdafx.h"
#include "61_computeparticles.h"
//-----------------------------------------------------------------------------
bool ComputeParticlesApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPosition(glm::vec3(0.0f, 0.0f, -4.0f));
	camera.setRotation(glm::vec3(0.0f));
	camera.setRotationSpeed(0.25f);
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 256.0f);

	// We will be using the queue family indices to check if graphics and compute queue families differ
		// If that's the case, we need additional barriers for acquiring and releasing resources
	graphics.queueFamilyIndex = m_vulkanDevice->queueFamilyIndices.graphics;
	compute.queueFamilyIndex = m_vulkanDevice->queueFamilyIndices.compute;
	loadAssets();
	setupDescriptorPool();
	prepareGraphics();
	prepareCompute();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void ComputeParticlesApp::OnDestroy()
{
	if (device) {
		// Graphics
		vkDestroyPipeline(device, graphics.pipeline, nullptr);
		vkDestroyPipelineLayout(device, graphics.pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, graphics.descriptorSetLayout, nullptr);
		vkDestroySemaphore(device, graphics.semaphore, nullptr);

		// Compute
		compute.uniformBuffer.destroy();
		vkDestroyPipelineLayout(device, compute.pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, compute.descriptorSetLayout, nullptr);
		vkDestroyPipeline(device, compute.pipeline, nullptr);
		vkDestroySemaphore(device, compute.semaphore, nullptr);
		vkDestroyCommandPool(device, compute.commandPool, nullptr);

		storageBuffer.destroy();
		textures.particle.destroy();
		textures.gradient.destroy();
	}
}
//-----------------------------------------------------------------------------
void ComputeParticlesApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void ComputeParticlesApp::OnFrame()
{
	draw();

	if (!attachToCursor)
	{
		if (animStart > 0.0f)
		{
			animStart -= frameTimer * 5.0f;
		}
		else if (animStart <= 0.0f)
		{
			timer += frameTimer * 0.04f;
			if (timer > 1.f)
				timer = 0.f;
		}
	}

	updateUniformBuffers();
}
//-----------------------------------------------------------------------------
void ComputeParticlesApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		overlay->checkBox("Attach attractor to cursor", &attachToCursor);
	}
}
//-----------------------------------------------------------------------------
void ComputeParticlesApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void ComputeParticlesApp::OnKeyPressed(uint32_t key)
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
void ComputeParticlesApp::OnKeyUp(uint32_t key)
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
void ComputeParticlesApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
