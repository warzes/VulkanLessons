#pragma once

// Using descriptor sets for passing data to shader stages

class DescriptorSets final : public EngineApp
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
	void getEnabledFeatures() final;
	void buildCommandBuffers() final;
	void loadAssets();
	void setupDescriptors();
	void preparePipelines();
	void prepareUniformBuffers();
	void updateUniformBuffers();
	void draw();

	Camera camera;

	bool animate = true;

	struct Cube {
		struct Matrices {
			glm::mat4 projection;
			glm::mat4 view;
			glm::mat4 model;
		} matrices;
		VkDescriptorSet descriptorSet;
		vks::Texture2D texture;
		vks::Buffer uniformBuffer;
		glm::vec3 rotation;
	};
	std::array<Cube, 2> cubes;

	vkglTF::Model model;

	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;

	VkDescriptorSetLayout descriptorSetLayout;
};