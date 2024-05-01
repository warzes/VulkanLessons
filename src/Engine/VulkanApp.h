#pragma once

#include "PlatformApp.h"
#include "VulkanInstance.h"
#include "VulkanAdapter.h"


#include "VulkanSwapChain.h"
#include "VulkanDevice.h"
#include "VulkanglTFModel.h"
#include "VulkanTexture.h"
#include "Benchmark.h"


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

	VulkanInstance m_instance{};
	VulkanAdapter m_adapter{};
	VulkanDevice* m_vulkanDevice = nullptr;
	// эти опции устанавливать в getEnabledFeatures или getEnabledExtensions
	VkPhysicalDeviceFeatures enabledFeatures{}; // Set of physical device features to be enabled for this example (must be set in the derived constructor)
	std::vector<const char*> enabledDeviceExtensions; // Set of device extensions to be enabled for this example (must be set in the derived constructor)
	void* deviceCreatepNextChain = nullptr; // Optional pNext structure for passing extension structures to device creation
	VkDevice device{ VK_NULL_HANDLE };
	VkQueue queue{ VK_NULL_HANDLE }; // Handle to the device graphics queue that command buffers are submitted to
	VkFormat depthFormat{}; // Depth buffer format (selected during Vulkan initialization)
	struct {
		// Swap chain image presentation
		VkSemaphore presentComplete{ VK_NULL_HANDLE };
		// Command buffer submission and execution
		VkSemaphore renderComplete{ VK_NULL_HANDLE };
	} semaphores; 	// Synchronization semaphores
	VkSubmitInfo submitInfo{}; // Contains command buffers and semaphores to be presented to the queue
	VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // @brief Pipeline stages used to wait at for graphics queue submissions
	VulkanSwapChain swapChain; // Wraps the swap chain to present images (framebuffers) to the windowing system
	VkCommandPool commandPool{ VK_NULL_HANDLE };
	std::vector<VkCommandBuffer> drawCommandBuffers; // Command buffers used for rendering
	std::vector<VkFence> waitFences;
	struct {
		VkImage image{};
		VkDeviceMemory memory{};
		VkImageView view{};
	} depthStencil{};// @brief Default depth stencil attachment used by the default render pass
	VkRenderPass renderPass{ VK_NULL_HANDLE }; // Global render pass for frame buffer writes
	VkPipelineCache pipelineCache{ VK_NULL_HANDLE };
	std::vector<VkFramebuffer> frameBuffers; // List of available frame buffers (same as number of swap chain images)
	uint32_t currentBuffer = 0; // Active frame buffer index
	std::vector<VkShaderModule> shaderModules; // List of shader modules created (stored for cleanup)
	VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
	
	bool requiresStencil{ false };
	VkClearColorValue defaultClearColor = { { 0.025f, 0.025f, 0.025f, 1.0f } };
	vks::Benchmark benchmark;
	bool resized = false;

private:
	bool initVulkan(const RenderSystemCreateInfo& createInfo);
	bool prepareRender(const RenderSystemCreateInfo& createInfo, bool fullscreen);
	void initSwapchain();
	void setupSwapChain(bool vsync, uint32_t* width, uint32_t* height);
	void createCommandPool();
	void createCommandBuffers();
	void createSynchronizationPrimitives();
	void createPipelineCache();
	void destroyCommandBuffers();

	bool m_vsync = false;
	bool prepared = false;
};