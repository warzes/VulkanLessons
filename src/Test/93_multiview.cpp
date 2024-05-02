#include "stdafx.h"
#include "93_multiview.h"
//-----------------------------------------------------------------------------
bool MultiviewApp::OnCreate()
{
	camera.type = Camera::CameraType::firstperson;
	camera.setRotation(glm::vec3(0.0f, 90.0f, 0.0f));
	camera.setTranslation(glm::vec3(7.0f, 3.2f, 0.0f));
	camera.movementSpeed = 5.0f;

	loadAssets();
	prepareMultiview();
	prepareUniformBuffers();
	prepareDescriptors();
	preparePipelines();

	VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(drawCommandBuffers.size()));
	multiviewPass.commandBuffers.resize(drawCommandBuffers.size());
	VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, multiviewPass.commandBuffers.data()));

	buildCommandBuffers();

	VkFenceCreateInfo fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	multiviewPass.waitFences.resize(multiviewPass.commandBuffers.size());
	for (auto& fence : multiviewPass.waitFences) {
		VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));
	}

	return true;
}
//-----------------------------------------------------------------------------
void MultiviewApp::OnDestroy()
{
	if (device) {
		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		vkDestroyImageView(device, multiviewPass.color.view, nullptr);
		vkDestroyImage(device, multiviewPass.color.image, nullptr);
		vkFreeMemory(device, multiviewPass.color.memory, nullptr);
		vkDestroyImageView(device, multiviewPass.depth.view, nullptr);
		vkDestroyImage(device, multiviewPass.depth.image, nullptr);
		vkFreeMemory(device, multiviewPass.depth.memory, nullptr);
		vkDestroyRenderPass(device, multiviewPass.renderPass, nullptr);
		vkDestroySampler(device, multiviewPass.sampler, nullptr);
		vkDestroyFramebuffer(device, multiviewPass.frameBuffer, nullptr);
		vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(multiviewPass.commandBuffers.size()), multiviewPass.commandBuffers.data());
		vkDestroySemaphore(device, multiviewPass.semaphore, nullptr);
		for (auto& fence : multiviewPass.waitFences) {
			vkDestroyFence(device, fence, nullptr);
		}
		for (auto& pipeline : viewDisplayPipelines) {
			vkDestroyPipeline(device, pipeline, nullptr);
		}
		uniformBuffer.destroy();
	}
}
//-----------------------------------------------------------------------------
void MultiviewApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void MultiviewApp::OnFrame()
{
	updateUniformBuffers();
	draw();
}
//-----------------------------------------------------------------------------
void MultiviewApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		if (overlay->sliderFloat("Eye separation", &eyeSeparation, -1.0f, 1.0f)) {
			updateUniformBuffers();
		}
		if (overlay->sliderFloat("Barrel distortion", &uniformData.distortionAlpha, -0.6f, 0.6f)) {
			updateUniformBuffers();
		}
	}
}
//-----------------------------------------------------------------------------
void MultiviewApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);

	windowResized();
}
//-----------------------------------------------------------------------------
void MultiviewApp::OnKeyPressed(uint32_t key)
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
void MultiviewApp::OnKeyUp(uint32_t key)
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
void MultiviewApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
