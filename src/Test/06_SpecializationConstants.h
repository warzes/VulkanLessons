#pragma once

// Uses SPIR-V specialization constants to create multiple pipelines with different lighting paths from a single "uber" shader.

class SpecializationConstants final : public EngineApp
{
public:
	EngineCreateInfo GetCreateInfo() const final { return {}; }
	bool OnCreate() final;
	void OnDestroy() final;
	void OnUpdate(float deltaTime) final;
	void OnFrame() final;

	void OnWindowResize(uint32_t destWidth, uint32_t destHeight) final;
	void OnKeyPressed(uint32_t key) final;
	void OnKeyUp(uint32_t key) final;
	void OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy) final;

private:
	void buildCommandBuffers() final;
	void loadAssets();
	void setupDescriptors();
	void preparePipelines();
	void prepareUniformBuffers();
	void updateUniformBuffers();
	void draw();

	Camera camera;

	vkglTF::Model scene;
	vks::Texture2D colormap;

	struct UniformData {
		glm::mat4 projection;
		glm::mat4 modelView;
		glm::vec4 lightPos{ 0.0f, -2.0f, 1.0f, 0.0f };
	} uniformData;
	VulkanBuffer uniformBuffer;

	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };

	struct Pipelines {
		VkPipeline phong{ VK_NULL_HANDLE };
		VkPipeline toon{ VK_NULL_HANDLE };
		VkPipeline textured{ VK_NULL_HANDLE };
	} pipelines;
};