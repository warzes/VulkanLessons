#pragma once

class PipelinesApp final : public EngineApp
{
public:
	EngineCreateInfo GetCreateInfo() const final { return {}; }
	bool OnCreate() final;
	void OnDestroy() final;
	void OnUpdate(float deltaTime) final;
	void OnFrame() final;
	void OnUpdateUIOverlay(vks::UIOverlay* overlay) final;

	void OnWindowResize(uint32_t destWidth, uint32_t destHeight) final;
	void OnKeyPressed(uint32_t key) final;
	void OnKeyUp(uint32_t key) final;
	void OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy) final;

private:

	// Enable physical device features required for this example
	void getEnabledFeatures() final;
	void buildCommandBuffers() final;

	void loadAssets();
	void setupDescriptors();
	void preparePipelines();
	// Prepare and initialize uniform buffer containing shader uniforms
	void prepareUniformBuffers();

	void updateUniformBuffers();

	void draw();

	vkglTF::Model scene;

	struct UniformData {
		glm::mat4 projection;
		glm::mat4 modelView;
		glm::vec4 lightPos{ 0.0f, 2.0f, 1.0f, 0.0f };
	} uniformData;
	VulkanBuffer uniformBuffer;

	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };

	struct {
		VkPipeline phong{ VK_NULL_HANDLE };
		VkPipeline wireframe{ VK_NULL_HANDLE };
		VkPipeline toon{ VK_NULL_HANDLE };
	} pipelines;

	Camera camera;
};