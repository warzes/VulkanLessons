#pragma once

// We want to keep GPU and CPU busy. To do that we may start building a new command buffer while the previous one is still being executed
// This number defines how many frames may be worked on simultaneously at once
// Increasing this number may improve performance but will also introduce additional latency
#define MAX_CONCURRENT_FRAMES 2

class TriangleApp final : public EngineApp
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
	void createSynchronizationPrimitives();
	void createCommandBuffers();
	void createVertexBuffer();
	void createUniformBuffers();
	void createDescriptorSetLayout();
	void createDescriptorPool();
	void createDescriptorSets();
	void createPipelines();
	VkShaderModule loadSPIRVShader(std::string filename);

	// Vertex layout used in this example
	struct Vertex {
		float position[3];
		float color[3];
	};

	// Vertex buffer and attributes
	struct {
		VkDeviceMemory memory{ VK_NULL_HANDLE }; // Handle to the device memory for this buffer
		VkBuffer buffer;						 // Handle to the Vulkan buffer object that the memory is bound to
	} vertices;

	// Index buffer
	struct {
		VkDeviceMemory memory{ VK_NULL_HANDLE };
		VkBuffer buffer;
		uint32_t count{ 0 };
	} indices;

	// Uniform buffer block object
	struct UniformBuffer {
		VkDeviceMemory memory;
		VkBuffer buffer;
		// The descriptor set stores the resources bound to the binding points in a shader
		// It connects the binding points of the different shaders with the buffers and images used for those bindings
		VkDescriptorSet descriptorSet;
		// We keep a pointer to the mapped buffer, so we can easily update it's contents via a memcpy
		uint8_t* mapped{ nullptr };
	};
	// We use one UBO per frame, so we can have a frame overlap and make sure that uniforms aren't updated while still in use
	std::array<UniformBuffer, MAX_CONCURRENT_FRAMES> uniformBuffers;

	// For simplicity we use the same uniform block layout as in the shader:
	//
	//	layout(set = 0, binding = 0) uniform UBO
	//	{
	//		mat4 projectionMatrix;
	//		mat4 modelMatrix;
	//		mat4 viewMatrix;
	//	} ubo;
	//
	// This way we can just memcopy the ubo data to the ubo
	// Note: You should use data types that align with the GPU in order to avoid manual padding (vec4, mat4)
	struct ShaderData {
		glm::mat4 projectionMatrix;
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
	};

	// The pipeline layout is used by a pipeline to access the descriptor sets
	// It defines interface (without binding any actual data) between the shader stages used by the pipeline and the shader resources
	// A pipeline layout can be shared among multiple pipelines as long as their interfaces match
	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };

	// Pipelines (often called "pipeline state objects") are used to bake all states that affect a pipeline
	// While in OpenGL every state can be changed at (almost) any time, Vulkan requires to layout the graphics (and compute) pipeline states upfront
	// So for each combination of non-dynamic pipeline states you need a new pipeline (there are a few exceptions to this not discussed here)
	// Even though this adds a new dimension of planning ahead, it's a great opportunity for performance optimizations by the driver
	VkPipeline pipeline{ VK_NULL_HANDLE };

	// The descriptor set layout describes the shader binding layout (without actually referencing descriptor)
	// Like the pipeline layout it's pretty much a blueprint and can be used with different descriptor sets as long as their layout matches
	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };

	// Synchronization primitives
	// Synchronization is an important concept of Vulkan that OpenGL mostly hid away. Getting this right is crucial to using Vulkan.

	// Semaphores are used to coordinate operations within the graphics queue and ensure correct command ordering
	std::array<VkSemaphore, MAX_CONCURRENT_FRAMES> presentCompleteSemaphores{};
	std::array<VkSemaphore, MAX_CONCURRENT_FRAMES> renderCompleteSemaphores{};

	VkCommandPool commandPool{ VK_NULL_HANDLE };
	std::array<VkCommandBuffer, MAX_CONCURRENT_FRAMES> commandBuffers{};
	std::array<VkFence, MAX_CONCURRENT_FRAMES> waitFences{};

	// To select the correct sync objects, we need to keep track of the current frame
	uint32_t currentFrame{ 0 };

	Camera camera;
};