#include "stdafx.h"
#include "62_computenbody.h"
//-----------------------------------------------------------------------------
bool ComputeNBodyApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 512.0f);
	camera.setRotation(glm::vec3(-26.0f, 75.0f, 0.0f));
	camera.setTranslation(glm::vec3(0.0f, 0.0f, -14.0f));
	camera.movementSpeed = 2.5f;

	// We will be using the queue family indices to check if graphics and compute queue families differ
		// If that's the case, we need additional barriers for acquiring and releasing resources
	graphics.queueFamilyIndex = m_vulkanDevice->queueFamilyIndices.graphics;
	compute.queueFamilyIndex = m_vulkanDevice->queueFamilyIndices.compute;
	loadAssets();
	prepareStorageBuffers();
	prepareGraphics();
	prepareCompute();

	return true;
}
//-----------------------------------------------------------------------------
void ComputeNBodyApp::OnDestroy()
{
	if (device) {
		// Graphics
		graphics.uniformBuffer.destroy();
		vkDestroyPipeline(device, graphics.pipeline, nullptr);
		vkDestroyPipelineLayout(device, graphics.pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, graphics.descriptorSetLayout, nullptr);
		vkDestroySemaphore(device, graphics.semaphore, nullptr);

		// Compute
		compute.uniformBuffer.destroy();
		vkDestroyPipelineLayout(device, compute.pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, compute.descriptorSetLayout, nullptr);
		vkDestroyPipeline(device, compute.pipelineCalculate, nullptr);
		vkDestroyPipeline(device, compute.pipelineIntegrate, nullptr);
		vkDestroySemaphore(device, compute.semaphore, nullptr);
		vkDestroyCommandPool(device, compute.commandPool, nullptr);

		storageBuffer.destroy();

		textures.particle.destroy();
		textures.gradient.destroy();
	}
}
//-----------------------------------------------------------------------------
void ComputeNBodyApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void ComputeNBodyApp::OnFrame()
{
	updateComputeUniformBuffers();
	updateGraphicsUniformBuffers();
	draw();
}
//-----------------------------------------------------------------------------
void ComputeNBodyApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{

}
//-----------------------------------------------------------------------------
void ComputeNBodyApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void ComputeNBodyApp::OnKeyPressed(uint32_t key)
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
void ComputeNBodyApp::OnKeyUp(uint32_t key)
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
void ComputeNBodyApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
