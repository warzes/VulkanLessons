#pragma once

/*
* This sample shows how load and render an cubemap array texture. A single image contains multiple cube maps.
* The cubemap to be displayed is selected in the fragment shader
*/

class TextureCubemapArrayApp final : public EngineApp
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
	// Loads a cubemap array from a file, uploads it to the device and create all Vulkan resources required to display it
	void loadCubemapArray(std::string filename, VkFormat format);

	void buildCommandBuffers() final;
	void loadAssets();
	void setupDescriptors();
	void preparePipelines();
	void prepareUniformBuffers();
	void updateUniformBuffers();
	void draw();

	Camera camera;

	bool displaySkybox = true;

	vks::Texture cubeMapArray;

	struct Meshes {
		vkglTF::Model skybox;
		std::vector<vkglTF::Model> objects;
		int32_t objectIndex = 0;
	} models;

	struct UniformData {
		glm::mat4 projection;
		glm::mat4 modelView;
		glm::mat4 inverseModelview;
		float lodBias = 0.0f;
		// Used by the fragment shader to select the cubemap from the array cubemap
		int cubeMapIndex = 1;
	} uniformData;
	vks::Buffer uniformBuffer;

	struct {
		VkPipeline skybox{ VK_NULL_HANDLE };
		VkPipeline reflect{ VK_NULL_HANDLE };
	} pipelines;

	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };

	std::vector<std::string> objectNames;
};