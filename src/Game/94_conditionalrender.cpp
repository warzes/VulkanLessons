#include "stdafx.h"
#include "94_conditionalrender.h"
//-----------------------------------------------------------------------------
bool ConditionalRenderApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPerspective(45.0f, (float)destWidth / (float)destHeight, 0.1f, 512.0f);
	camera.setRotation(glm::vec3(-2.25f, -52.0f, 0.0f));
	camera.setTranslation(glm::vec3(1.9f, -2.05f, -18.0f));
	camera.rotationSpeed *= 0.25f;

	loadAssets();
	prepareConditionalRendering();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void ConditionalRenderApp::OnDestroy()
{
	if (device) {
		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		uniformBuffer.destroy();
		conditionalBuffer.destroy();
	}
}
//-----------------------------------------------------------------------------
void ConditionalRenderApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void ConditionalRenderApp::OnFrame()
{
	updateUniformBuffers();
	draw();
}
//-----------------------------------------------------------------------------
void ConditionalRenderApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Visibility")) {

		if (overlay->button("All")) {
			for (auto i = 0; i < conditionalVisibility.size(); i++) {
				conditionalVisibility[i] = 1;
			}
			updateConditionalBuffer();
		}
		ImGui::SameLine();
		if (overlay->button("None")) {
			for (auto i = 0; i < conditionalVisibility.size(); i++) {
				conditionalVisibility[i] = 0;
			}
			updateConditionalBuffer();
		}
		ImGui::NewLine();

		ImGui::BeginChild("InnerRegion", ImVec2(200.0f * overlay->scale, 400.0f * overlay->scale), false);
		for (auto node : scene.linearNodes) {
			// Add visibility toggle checkboxes for all model nodes with a mesh
			if (node->mesh) {
				if (overlay->checkBox(("[" + std::to_string(node->index) + "] " + node->mesh->name).c_str(), &conditionalVisibility[node->index])) {
					updateConditionalBuffer();
				}
			}
		}
		ImGui::EndChild();

	}
}
//-----------------------------------------------------------------------------
void ConditionalRenderApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void ConditionalRenderApp::OnKeyPressed(uint32_t key)
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
void ConditionalRenderApp::OnKeyUp(uint32_t key)
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
void ConditionalRenderApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
