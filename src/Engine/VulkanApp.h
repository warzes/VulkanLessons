#pragma once

#include "PlatformApp.h"
#include "VulkanSwapChain.h"
#include "VulkanDevice.h"
#include "VulkanglTFModel.h"
#include "VulkanTexture.h"
#include "Benchmark.h"
#include "VulkanInstance.h"

struct RenderSystemCreateInfo final
{
	std::vector<const char*> enabledInstanceExtensions;
	bool validationLayers = false;
	bool vsync = false;

	struct
	{
		uint32_t subpass = 0;
	} overlay;
	bool requiresStencil = false;
};

class VulkanApp : public PlatformApp
{
public:
	virtual ~VulkanApp() = default;

	std::string GetDeviceName() const;
	// This function is used to request a device memory type that supports all the property flags we request (e.g. device local, host visible)
	// Upon success it will return the index of the memory type that fits our requested memory properties
	// This is necessary as implementations can offer an arbitrary number of memory types with different memory properties.
	// You can check https://vulkan.gpuinfo.org/ for details on different memory configurations
	uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties);

	VkDevice& GetDevice() { return device; }
	VulkanSwapChain& GetSwapChain() { return swapChain; }
	VkQueue& GetQueue() { return queue; }
	VkDescriptorPool& GetDescriptorPool() { return descriptorPool; }
	VkRenderPass& GetRenderPass() { return renderPass; }
	VkPipelineCache& GetPipelineCache() { return pipelineCache; }
	std::vector<VkFramebuffer>& GetFrameBuffers() { return frameBuffers; }

	uint32_t destWidth;// TODO: временно
	uint32_t destHeight;// TODO: временно

	bool& RenderPrepared() { return prepared; }

	void DrawUI(const VkCommandBuffer commandBuffer);

protected:
	bool initVulkanApp(const RenderSystemCreateInfo& createInfo, bool fullscreen);
	virtual void setEnabledInstanceExtensions() {} // TODO: все эти опции передавать через CreateInfo. функцию удалить
	// Called after the physical device features have been read, can be used to set features to enable on the device
	virtual void getEnabledFeatures() {} // TODO: если нужно
	// Called after the physical device extensions have been read, can be used to enable extensions based on the supported extension listing
	virtual void getEnabledExtensions() {} // TODO: если нужно
	virtual void setupDepthStencil(uint32_t width, uint32_t height);
	virtual void setupRenderPass();
	virtual void setupFrameBuffer(uint32_t width, uint32_t height);
	virtual void buildCommandBuffers() {}
	VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage);
	void renderFinal();
	void closeVulkanApp();
	void resizeRender(uint32_t destWidth, uint32_t destHeight);

	/** Prepare the next frame for workload submission by acquiring the next swap chain image */
	void prepareFrame();
	/** @brief Presents the current image to the swap chain */
	void submitFrame();
	/** @brief (Virtual) Default image acquire + submission and command buffer submission function */
	virtual void renderFrame();

	VulkanInstance m_instance;

	// Physical device (GPU) that Vulkan will use
	VkPhysicalDevice m_physicalDevice{ VK_NULL_HANDLE };
	/** @brief Encapsulated physical and logical vulkan device */
	vks::VulkanDevice* m_vulkanDevice;

	// Stores physical device properties (for e.g. checking device limits)
	VkPhysicalDeviceProperties deviceProperties{};
	// Stores the features available on the selected physical device (for e.g. checking if a feature is available)
	VkPhysicalDeviceFeatures deviceFeatures{};
	// Stores all available memory (type) properties for the physical device
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties{};
	/** @brief Set of physical device features to be enabled for this example (must be set in the derived constructor) */
	VkPhysicalDeviceFeatures enabledFeatures{};
	/** @brief Set of device extensions to be enabled for this example (must be set in the derived constructor) */
	std::vector<const char*> enabledDeviceExtensions;
	/** @brief Optional pNext structure for passing extension structures to device creation */
	void* deviceCreatepNextChain = nullptr;
	/** @brief Logical device, application's view of the physical device (GPU) */
	VkDevice device{ VK_NULL_HANDLE }; // TODO: копия из m_vulkanDevice
	// Handle to the device graphics queue that command buffers are submitted to
	VkQueue queue{ VK_NULL_HANDLE };
	// Depth buffer format (selected during Vulkan initialization)
	VkFormat depthFormat;
	// Command buffer pool
	VkCommandPool cmdPool{ VK_NULL_HANDLE };
	/** @brief Pipeline stages used to wait at for graphics queue submissions */
	VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	// Contains command buffers and semaphores to be presented to the queue
	VkSubmitInfo submitInfo;
	// Command buffers used for rendering
	std::vector<VkCommandBuffer> drawCmdBuffers;

	// Global render pass for frame buffer writes
	VkRenderPass renderPass{ VK_NULL_HANDLE };
	// List of available frame buffers (same as number of swap chain images)
	std::vector<VkFramebuffer> frameBuffers;
	// Active frame buffer index
	uint32_t currentBuffer = 0;
	// Descriptor set pool
	VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
	// List of shader modules created (stored for cleanup)
	std::vector<VkShaderModule> shaderModules;
	// Pipeline cache object
	VkPipelineCache pipelineCache{ VK_NULL_HANDLE };
	// Wraps the swap chain to present images (framebuffers) to the windowing system
	VulkanSwapChain swapChain;
	// Synchronization semaphores
	struct {
		// Swap chain image presentation
		VkSemaphore presentComplete;
		// Command buffer submission and execution
		VkSemaphore renderComplete;
	} semaphores;
	std::vector<VkFence> waitFences;
	bool requiresStencil{ false };

	VkClearColorValue defaultClearColor = { { 0.025f, 0.025f, 0.025f, 1.0f } };

	/** @brief Default depth stencil attachment used by the default render pass */
	struct {
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
	} depthStencil{};

	vks::Benchmark benchmark;

	bool resized = false;

private:
	bool initVulkan(const RenderSystemCreateInfo& createInfo);
	bool selectPhysicalDevice();
	bool prepareRender(const RenderSystemCreateInfo& createInfo, bool fullscreen);
	void initSwapchain();
	void createCommandPool();
	void setupSwapChain(bool vsync, uint32_t* width, uint32_t* height, bool fullscreen);
	void createCommandBuffers();
	void createSynchronizationPrimitives();
	void createPipelineCache();
	void destroyCommandBuffers();

	bool m_vsync = false;
	bool prepared = false;
};