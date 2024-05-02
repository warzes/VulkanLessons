#include "stdafx.h"
#include "63_computecullandlod.h"
//-----------------------------------------------------------------------------
bool ComputeCullAndLodApp::OnCreate()
{
	camera.type = Camera::CameraType::firstperson;
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 512.0f);
	camera.setTranslation(glm::vec3(0.5f, 0.0f, 0.0f));
	camera.movementSpeed = 5.0f;
	memset(&indirectStats, 0, sizeof(indirectStats));

	loadAssets();
	prepareBuffers();
	setupDescriptors();
	preparePipelines();
	prepareCompute();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void ComputeCullAndLodApp::OnDestroy()
{
	if (device) {
		vkDestroyPipeline(device, pipelines.plants, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		instanceBuffer.destroy();
		indirectCommandsBuffer.destroy();
		uniformData.scene.destroy();
		indirectDrawCountBuffer.destroy();
		compute.lodLevelsBuffers.destroy();
		vkDestroyPipelineLayout(device, compute.pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, compute.descriptorSetLayout, nullptr);
		vkDestroyPipeline(device, compute.pipeline, nullptr);
		vkDestroyFence(device, compute.fence, nullptr);
		vkDestroyCommandPool(device, compute.commandPool, nullptr);
		vkDestroySemaphore(device, compute.semaphore, nullptr);
	}
}
//-----------------------------------------------------------------------------
void ComputeCullAndLodApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void ComputeCullAndLodApp::OnFrame()
{
	updateUniformBuffer();
	draw();
}
//-----------------------------------------------------------------------------
void ComputeCullAndLodApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		overlay->checkBox("Freeze frustum", &fixedFrustum);
	}
	if (overlay->header("Statistics")) {
		overlay->text("Visible objects: %d", indirectStats.drawCount);
		for (uint32_t i = 0; i < MAX_LOD_LEVEL + 1; i++) {
			overlay->text("LOD %d: %d", i, indirectStats.lodCount[i]);
		}
	}
}
//-----------------------------------------------------------------------------
void ComputeCullAndLodApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void ComputeCullAndLodApp::OnKeyPressed(uint32_t key)
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
void ComputeCullAndLodApp::OnKeyUp(uint32_t key)
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
void ComputeCullAndLodApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
