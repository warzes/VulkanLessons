#include "stdafx.h"
#include "40_multithreading.h"
//-----------------------------------------------------------------------------
bool MultithreadingApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPosition(glm::vec3(0.0f, -0.0f, -32.5f));
	camera.setRotation(glm::vec3(0.0f));
	camera.setRotationSpeed(0.5f);
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 256.0f);
	// Get number of max. concurrent threads
	numThreads = std::thread::hardware_concurrency();
	assert(numThreads > 0);
#if defined(__ANDROID__)
	LOGD("numThreads = %d", numThreads);
#else
	std::cout << "numThreads = " << numThreads << std::endl;
#endif
	threadPool.setThreadCount(numThreads);
	numObjectsPerThread = 512 / numThreads;
	rndEngine.seed(benchmark.active ? 0 : (unsigned)time(nullptr));

	// Create a fence for synchronization
	VkFenceCreateInfo fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	vkCreateFence(device, &fenceCreateInfo, nullptr, &renderFence);
	loadAssets();
	preparePipelines();
	prepareMultiThreadedRenderer();
	updateMatrices();

	return true;
}
//-----------------------------------------------------------------------------
void MultithreadingApp::OnDestroy()
{
	if (device) {
		vkDestroyPipeline(device, pipelines.phong, nullptr);
		vkDestroyPipeline(device, pipelines.starsphere, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		for (auto& thread : threadData) {
			vkFreeCommandBuffers(device, thread.commandPool, static_cast<uint32_t>(thread.commandBuffer.size()), thread.commandBuffer.data());
			vkDestroyCommandPool(device, thread.commandPool, nullptr);
		}
		vkDestroyFence(device, renderFence, nullptr);
	}
}
//-----------------------------------------------------------------------------
void MultithreadingApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void MultithreadingApp::OnFrame()
{
	updateMatrices();
	draw();
}
//-----------------------------------------------------------------------------
void MultithreadingApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Statistics")) {
		overlay->text("Active threads: %d", numThreads);
	}
	if (overlay->header("Settings")) {
		overlay->checkBox("Stars", &displayStarSphere);
	}
}
//-----------------------------------------------------------------------------
void MultithreadingApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void MultithreadingApp::OnKeyPressed(uint32_t key)
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
void MultithreadingApp::OnKeyUp(uint32_t key)
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
void MultithreadingApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
