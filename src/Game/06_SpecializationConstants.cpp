#include "stdafx.h"
#include "06_SpecializationConstants.h"
//-----------------------------------------------------------------------------
bool SpecializationConstants::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPerspective(60.0f, ((float)destWidth / 3.0f) / (float)destHeight, 0.1f, 512.0f);
	camera.setRotation(glm::vec3(-40.0f, -90.0f, 0.0f));
	camera.setTranslation(glm::vec3(0.0f, 0.0f, -2.0f));

	return true;
}
//-----------------------------------------------------------------------------
void SpecializationConstants::OnDestroy()
{
	if (device) {
		vkDestroyPipeline(device, pipelines.phong, nullptr);
		vkDestroyPipeline(device, pipelines.textured, nullptr);
		vkDestroyPipeline(device, pipelines.toon, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		colormap.destroy();
		uniformBuffer.destroy();
	}
}
//-----------------------------------------------------------------------------
void SpecializationConstants::OnUpdate(float deltaTime)
{
}
//-----------------------------------------------------------------------------
void SpecializationConstants::OnFrame()
{
	if (!RenderPrepared())
		return;

}
//-----------------------------------------------------------------------------
void SpecializationConstants::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void SpecializationConstants::OnKeyPressed(uint32_t key)
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
void SpecializationConstants::OnKeyUp(uint32_t key)
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
void SpecializationConstants::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
void SpecializationConstants::buildCommandBuffers()
{
	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	VkClearValue clearValues[2];
	clearValues[0].color = defaultClearColor;
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = destWidth;
	renderPassBeginInfo.renderArea.extent.height = destHeight;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
	{
		// Set target frame buffer
		renderPassBeginInfo.framebuffer = frameBuffers[i];

		VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

		vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkRect2D scissor = vks::initializers::rect2D(destWidth, destHeight, 0, 0);
		vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

		vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

		// Left
		VkViewport viewport = vks::initializers::viewport((float)destWidth / 3.0f, (float)destHeight, 0.0f, 1.0f);
		vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
		vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.phong);
		scene.draw(drawCmdBuffers[i]);

		// Center
		viewport.x = (float)destWidth / 3.0f;
		vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
		vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.toon);
		scene.draw(drawCmdBuffers[i]);

		// Right
		viewport.x = (float)destWidth / 3.0f + (float)destWidth / 3.0f;
		vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
		vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.textured);
		scene.draw(drawCmdBuffers[i]);

		drawUI(drawCmdBuffers[i]);

		vkCmdEndRenderPass(drawCmdBuffers[i]);

		VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
	}
}
//-----------------------------------------------------------------------------
void SpecializationConstants::loadAssets()
{
	scene.loadFromFile(getAssetPath() + "models/color_teapot_spheres.gltf", vulkanDevice, queue, vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY);
	colormap.loadFromFile(getAssetPath() + "textures/metalplate_nomips_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue);
}
//-----------------------------------------------------------------------------