#pragma once

/*
* Vulkan Example - Texture arrays and instanced rendering
*
* This sample shows how to load and render a texture array. This is a single layered texture where each layer contains different image data.
* The different layers are displayed on cubes using instancing, where each instance selects a different layer from the texture
*/

class TextureArraysApp final : public EngineApp
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
	void loadTextureArray(std::string filename, VkFormat format);
	void loadAssets();
	void buildCommandBuffers() final;
	// Creates a vertex and index buffer for a cube
	// This is used to display the texture on
	void generateCube();

	void setupDescriptors();

	void preparePipelines();
	void prepareUniformBuffers();
	void updateUniformBuffersCamera();
	void draw();

	Camera camera;

#define MAX_LAYERS 8

	// Vertex layout for this example
	struct Vertex {
		float pos[3];
		float uv[2];
	};

	// Number of array layers in texture array
	// Also used as instance count
	uint32_t layerCount{ 0 };
	vks::Texture textureArray;

	vks::Buffer vertexBuffer;
	vks::Buffer indexBuffer;
	uint32_t indexCount{ 0 };

	// Values passed to the shader per drawn instance
	struct alignas(16) PerInstanceData {
		// Model matrix
		glm::mat4 model;
		// Layer index from which this instance will sample in the fragment shader
		float arrayIndex{ 0 };
	};

	struct UniformData {
		// Global matrices
		struct {
			glm::mat4 projection;
			glm::mat4 view;
		} matrices;
		// Separate data for each instance
		PerInstanceData* instance{ nullptr };
	} uniformData;
	vks::Buffer uniformBuffer;

	VkPipeline pipeline{ VK_NULL_HANDLE };
	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
};