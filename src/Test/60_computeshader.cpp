#include "stdafx.h"
#include "60_computeshader.h"
//-----------------------------------------------------------------------------
bool ComputeShaderApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPosition(glm::vec3(0.0f, 0.0f, -2.0f));
	camera.setRotation(glm::vec3(0.0f));
	camera.setPerspective(60.0f, (float)destWidth * 0.5f / (float)destHeight, 1.0f, 256.0f);

	loadAssets();
	generateQuad();
	prepareUniformBuffers();
	prepareStorageImage();
	setupDescriptorPool();
	prepareGraphics();
	prepareCompute();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void ComputeShaderApp::OnDestroy()
{
	if (device) {
		// Graphics
		vkDestroyPipeline(device, graphics.pipeline, nullptr);
		vkDestroyPipelineLayout(device, graphics.pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, graphics.descriptorSetLayout, nullptr);
		vkDestroySemaphore(device, graphics.semaphore, nullptr);
		graphics.uniformBuffer.destroy();

		// Compute
		for (auto& pipeline : compute.pipelines)
		{
			vkDestroyPipeline(device, pipeline, nullptr);
		}
		vkDestroyPipelineLayout(device, compute.pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, compute.descriptorSetLayout, nullptr);
		vkDestroySemaphore(device, compute.semaphore, nullptr);
		vkDestroyCommandPool(device, compute.commandPool, nullptr);

		vertexBuffer.destroy();
		indexBuffer.destroy();

		textureColorMap.destroy();
		storageImage.destroy();
	}
}
//-----------------------------------------------------------------------------
void ComputeShaderApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void ComputeShaderApp::OnFrame()
{
	draw();
	updateUniformBuffers();
}
//-----------------------------------------------------------------------------
void ComputeShaderApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		if (overlay->comboBox("Shader", &compute.pipelineIndex, filterNames)) {
			buildComputeCommandBuffer();
		}
	}
}
//-----------------------------------------------------------------------------
void ComputeShaderApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void ComputeShaderApp::OnKeyPressed(uint32_t key)
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
void ComputeShaderApp::OnKeyUp(uint32_t key)
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
void ComputeShaderApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
