#include "stdafx.h"
#include "90_conservativeraster.h"
//-----------------------------------------------------------------------------
bool ConservativerasterApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 512.0f);
	camera.setRotation(glm::vec3(0.0f));
	camera.setTranslation(glm::vec3(0.0f, 0.0f, -2.0f));

	loadAssets();
	prepareOffscreen();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void ConservativerasterApp::OnDestroy()
{
	if (device) {
		vkDestroyImageView(device, offscreenPass.color.view, nullptr);
		vkDestroyImage(device, offscreenPass.color.image, nullptr);
		vkFreeMemory(device, offscreenPass.color.mem, nullptr);
		vkDestroyImageView(device, offscreenPass.depth.view, nullptr);
		vkDestroyImage(device, offscreenPass.depth.image, nullptr);
		vkFreeMemory(device, offscreenPass.depth.mem, nullptr);

		vkDestroyRenderPass(device, offscreenPass.renderPass, nullptr);
		vkDestroySampler(device, offscreenPass.sampler, nullptr);
		vkDestroyFramebuffer(device, offscreenPass.frameBuffer, nullptr);

		vkDestroyPipeline(device, pipelines.triangle, nullptr);
		vkDestroyPipeline(device, pipelines.triangleOverlay, nullptr);
		vkDestroyPipeline(device, pipelines.triangleConservativeRaster, nullptr);
		vkDestroyPipeline(device, pipelines.fullscreen, nullptr);

		vkDestroyPipelineLayout(device, pipelineLayouts.fullscreen, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayouts.scene, nullptr);

		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.scene, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.fullscreen, nullptr);

		uniformBuffer.destroy();
		triangle.vertices.destroy();
		triangle.indices.destroy();
	}
}
//-----------------------------------------------------------------------------
void ConservativerasterApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void ConservativerasterApp::OnFrame()
{
	updateUniformBuffersScene();
	draw();
}
//-----------------------------------------------------------------------------
void ConservativerasterApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		if (overlay->checkBox("Conservative rasterization", &conservativeRasterEnabled)) {
			buildCommandBuffers();
		}
	}
	if (overlay->header("Device properties")) {
		overlay->text("maxExtraPrimitiveOverestimationSize: %f", conservativeRasterProps.maxExtraPrimitiveOverestimationSize);
		overlay->text("extraPrimitiveOverestimationSizeGranularity: %f", conservativeRasterProps.extraPrimitiveOverestimationSizeGranularity);
		overlay->text("primitiveUnderestimation:  %s", conservativeRasterProps.primitiveUnderestimation ? "yes" : "no");
		overlay->text("conservativePointAndLineRasterization:  %s", conservativeRasterProps.conservativePointAndLineRasterization ? "yes" : "no");
		overlay->text("degenerateTrianglesRasterized: %s", conservativeRasterProps.degenerateTrianglesRasterized ? "yes" : "no");
		overlay->text("degenerateLinesRasterized: %s", conservativeRasterProps.degenerateLinesRasterized ? "yes" : "no");
		overlay->text("fullyCoveredFragmentShaderInputVariable: %s", conservativeRasterProps.fullyCoveredFragmentShaderInputVariable ? "yes" : "no");
		overlay->text("conservativeRasterizationPostDepthCoverage: %s", conservativeRasterProps.conservativeRasterizationPostDepthCoverage ? "yes" : "no");
	}
}
//-----------------------------------------------------------------------------
void ConservativerasterApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void ConservativerasterApp::OnKeyPressed(uint32_t key)
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
void ConservativerasterApp::OnKeyUp(uint32_t key)
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
void ConservativerasterApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
