#pragma once

/*
* Vulkan Example - Texture loading (and display) example (including mip maps)
*
* This sample shows how to upload a 2D texture to the device and how to display it. In Vulkan this is done using images, views and samplers.
*/


class TextureApp final : public EngineApp
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
	void OnUpdateUIOverlay(vks::UIOverlay* overlay) final;

private:
	void getEnabledFeatures() final;

	// Vertex layout for this example
	struct Vertex {
		float pos[3];
		float uv[2];
		float normal[3];
	};

	// Contains all Vulkan objects that are required to store and use a texture
	// Note that this repository contains a texture class (VulkanTexture.hpp) that encapsulates texture loading functionality in a class that is used in subsequent demos
	struct Texture {
		VkSampler sampler{ VK_NULL_HANDLE };
		VkImage image{ VK_NULL_HANDLE };
		VkImageLayout imageLayout;
		VkDeviceMemory deviceMemory{ VK_NULL_HANDLE };
		VkImageView view{ VK_NULL_HANDLE };
		uint32_t width{ 0 };
		uint32_t height{ 0 };
		uint32_t mipLevels{ 0 };
	} texture;


	/*
		Upload texture image data to the GPU

		Vulkan offers two types of image tiling (memory layout):

		Linear tiled images:
			These are stored as is and can be copied directly to. But due to the linear nature they're not a good match for GPUs and format and feature support is very limited.
			It's not advised to use linear tiled images for anything else than copying from host to GPU if buffer copies are not an option.
			Linear tiling is thus only implemented for learning purposes, one should always prefer optimal tiled image.

		Optimal tiled images:
			These are stored in an implementation specific layout matching the capability of the hardware. They usually support more formats and features and are much faster.
			Optimal tiled images are stored on the device and not accessible by the host. So they can't be written directly to (like liner tiled images) and always require
			some sort of data copy, either from a buffer or	a linear tiled image.

		In Short: Always use optimal tiled images for rendering.
	*/
	void loadTexture();
	// Free all Vulkan resources used by a texture object
	void destroyTextureImage(Texture texture);
	void buildCommandBuffers();
	// Creates a vertex and index buffer for a quad made of two triangles
	// This is used to display the texture on
	void generateQuad();
	void setupDescriptors();
	void preparePipelines();
	// Prepare and initialize uniform buffer containing shader uniforms
	void prepareUniformBuffers();
	void updateUniformBuffers();
	void draw();

	Camera camera;
	
	vks::Buffer vertexBuffer;
	vks::Buffer indexBuffer;
	uint32_t indexCount{ 0 };

	struct UniformData {
		glm::mat4 projection;
		glm::mat4 modelView;
		glm::vec4 viewPos;
		// This is used to change the bias for the level-of-detail (mips) in the fragment shader
		float lodBias = 0.0f;
	} uniformData;
	vks::Buffer uniformBuffer;

	VkPipeline pipeline{ VK_NULL_HANDLE };
	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
};