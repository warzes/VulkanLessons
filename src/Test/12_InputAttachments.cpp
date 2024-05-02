#include "stdafx.h"
#include "12_InputAttachments.h"
//-----------------------------------------------------------------------------
EngineCreateInfo InputAttachmentsApp::GetCreateInfo() const
{
	EngineCreateInfo ci{};
	ci.render.overlay.subpass = 1;
	return ci;
}
//-----------------------------------------------------------------------------
bool InputAttachmentsApp::OnCreate()
{
	camera.type = Camera::CameraType::firstperson;
	camera.movementSpeed = 2.5f;
	camera.setPosition(glm::vec3(1.65f, 1.75f, -6.15f));
	camera.setRotation(glm::vec3(-12.75f, 380.0f, 0.0f));
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 256.0f);

	loadAssets();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void InputAttachmentsApp::OnDestroy()
{
	if (device) {
		for (uint32_t i = 0; i < attachments.size(); i++) {
			vkDestroyImageView(device, attachments[i].color.view, nullptr);
			vkDestroyImage(device, attachments[i].color.image, nullptr);
			vkFreeMemory(device, attachments[i].color.memory, nullptr);
			vkDestroyImageView(device, attachments[i].depth.view, nullptr);
			vkDestroyImage(device, attachments[i].depth.image, nullptr);
			vkFreeMemory(device, attachments[i].depth.memory, nullptr);
		}

		vkDestroyPipeline(device, pipelines.attachmentRead, nullptr);
		vkDestroyPipeline(device, pipelines.attachmentWrite, nullptr);

		vkDestroyPipelineLayout(device, pipelineLayouts.attachmentWrite, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayouts.attachmentRead, nullptr);

		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.attachmentWrite, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.attachmentRead, nullptr);

		uniformBuffers.matrices.destroy();
		uniformBuffers.params.destroy();
	}
}
//-----------------------------------------------------------------------------
void InputAttachmentsApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void InputAttachmentsApp::OnFrame()
{
	updateUniformBuffers();
	draw();
}
//-----------------------------------------------------------------------------
void InputAttachmentsApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		overlay->text("Input attachment");
		if (overlay->comboBox("##attachment", &uboParams.attachmentIndex, { "color", "depth" })) {
			updateUniformBuffers();
		}
		switch (uboParams.attachmentIndex) {
		case 0:
			overlay->text("Brightness");
			if (overlay->sliderFloat("##b", &uboParams.brightnessContrast[0], 0.0f, 2.0f)) {
				updateUniformBuffers();
			}
			overlay->text("Contrast");
			if (overlay->sliderFloat("##c", &uboParams.brightnessContrast[1], 0.0f, 4.0f)) {
				updateUniformBuffers();
			}
			break;
		case 1:
			overlay->text("Visible range");
			if (overlay->sliderFloat("min", &uboParams.range[0], 0.0f, uboParams.range[1])) {
				updateUniformBuffers();
			}
			if (overlay->sliderFloat("max", &uboParams.range[1], uboParams.range[0], 1.0f)) {
				updateUniformBuffers();
			}
			break;
		}
	}
}
//-----------------------------------------------------------------------------
void InputAttachmentsApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void InputAttachmentsApp::OnKeyPressed(uint32_t key)
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
void InputAttachmentsApp::OnKeyUp(uint32_t key)
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
void InputAttachmentsApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
