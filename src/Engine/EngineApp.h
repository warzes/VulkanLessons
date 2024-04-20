#pragma once

#include "VulkanSwapChain.h"
#include "VulkanDevice.h"
#include "VulkanUIOverlay.h"
#include "VulkanglTFModel.h"
#include "VulkanTexture.h"
#include "Benchmark.h"

struct WindowSystemCreateInfo final
{
	int width = 1024;
	int height = 768;
	const wchar_t* title = L"Game";
	bool fullscreen = false;
};

struct RenderSystemCreateInfo final
{
	std::vector<const char*> enabledInstanceExtensions;
	bool validation = false;
	bool vsync = false;

	struct
	{
		uint32_t subpass = 0;
	} overlay;
};

struct EngineCreateInfo final
{
	WindowSystemCreateInfo window;
	RenderSystemCreateInfo render;
};

struct EngineAppImpl;

class EngineApp
{
public:
	EngineApp();
	virtual ~EngineApp();

	void Run();

	virtual EngineCreateInfo GetCreateInfo() const { return {}; }
	virtual bool OnCreate() = 0;
	virtual void OnDestroy() = 0;
	virtual void OnUpdate(float deltaTime) = 0;
	virtual void OnFrame() = 0;
	virtual void OnWindowResize(uint32_t destWidth, uint32_t destHeight) {}
	/** @brief (Virtual) Called after a key was pressed, can be used to do custom key handling */
	virtual void OnKeyPressed(uint32_t) {}
	virtual void OnKeyUp(uint32_t) {}
	/** @brief (Virtual) Called after the mouse cursor moved and before internal events (like camera rotation) is handled */
	virtual void OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy) {}
	/** @brief (Virtual) Called when the UI overlay is updating, can be used to add custom elements to the overlay */
	virtual void OnUpdateUIOverlay(vks::UIOverlay* /*overlay*/) {}

	void Exit();
	float GetDeltaTime() const;
	bool& RenderPrepared() { return prepared; }

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

	bool& Paused() { return paused; }
	vks::UIOverlay& GetUIOverlay() { return UIOverlay; }

	/** @brief State of mouse/touch input */
	struct {
		struct {
			bool left = false;
			bool right = false;
			bool middle = false;
		} buttons;
		glm::vec2 position;
	} mouseState; // TODO: временно паблик
	void handleMouseMove(int32_t x, int32_t y);// TODO: временно паблик
	bool resizing = false;// TODO: временно
	uint32_t destWidth;// TODO: временно
	uint32_t destHeight;// TODO: временно
	void windowResize(uint32_t destWidth, uint32_t destHeight);

protected:
	/** @brief Adds the drawing commands for the ImGui overlay to the given command buffer */
	void drawUI(const VkCommandBuffer commandBuffer);
	/** @brief Loads a SPIR-V shader file for the given shader stage */
	VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage);

	/** Prepare the next frame for workload submission by acquiring the next swap chain image */
	void prepareFrame();
	/** @brief Presents the current image to the swap chain */
	void submitFrame();
	/** @brief (Virtual) Default image acquire + submission and command buffer submission function */
	virtual void renderFrame();

	// Frame counter to display fps
	uint32_t frameCounter = 0;
	uint32_t lastFPS = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp, tPrevEnd;

	// Vulkan instance, stores all per-application states
	VkInstance instance{ VK_NULL_HANDLE };
	std::vector<std::string> supportedInstanceExtensions;
	// Physical device (GPU) that Vulkan will use
	VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };

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
	std::vector<const char*> enabledInstanceExtensions;
	/** @brief Optional pNext structure for passing extension structures to device creation */
	void* deviceCreatepNextChain = nullptr;
	/** @brief Logical device, application's view of the physical device (GPU) */
	VkDevice device{ VK_NULL_HANDLE };
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

	/** @brief Encapsulated physical and logical vulkan device */
	vks::VulkanDevice* vulkanDevice;

	bool paused = false;

	/** @brief Last frame time measured using a high performance timer (if available) */
	float frameTimer = 1.0f;

	vks::Benchmark benchmark;

private:
	bool create();
	void setupDPIAwareness();
	bool initWindow(const WindowSystemCreateInfo& createInfo);
	bool initVulkan(const RenderSystemCreateInfo& createInfo);
	VkResult createInstance(bool enableValidation);
	/** @brief (Virtual) Called after the physical device features have been read, can be used to set features to enable on the device */
	virtual void getEnabledFeatures() {} // TODO: если нужно
	/** @brief (Virtual) Called after the physical device extensions have been read, can be used to enable extensions based on the supported extension listing*/
	virtual void getEnabledExtensions() {} // TODO: если нужно
	bool prepareRender(const RenderSystemCreateInfo& createInfo, bool fullscreen);
	void initSwapchain();
	void createCommandPool();
	void setupSwapChain(bool vsync, uint32_t* width, uint32_t* height, bool fullscreen);
	void createCommandBuffers();
	void createSynchronizationPrimitives();
	/** @brief (Virtual) Setup default depth and stencil views */
	virtual void setupDepthStencil(uint32_t width, uint32_t height);
	/** @brief (Virtual) Setup a default renderpass */
	virtual void setupRenderPass();
	void createPipelineCache();
	/** @brief (Virtual) Setup default framebuffers for all requested swapchain images */
	virtual void setupFrameBuffer(uint32_t width, uint32_t height);
	
	bool isEnd() const;
	void frame();
	void nextFrame();
	void render();
	void updateOverlay();

	/** @brief (Virtual) Called when resources have been recreated that require a rebuild of the command buffers (e.g. frame buffer), to be implemented by the sample application */
	virtual void buildCommandBuffers() {}
	void renderFinal();
	void destroy();
	void renderDestroy();
	void destroyCommandBuffers();

	std::string getWindowTitle();



	EngineAppImpl* m_data = nullptr;


	bool prepared = false;
	bool resized = false;
	bool validation = false;
	bool overlay = true;

	vks::UIOverlay UIOverlay;

	bool m_vsync = false;
	bool m_fullscreen = false;

	// Defines a frame rate independent timer value clamped from -1.0...1.0
	// For use in animations, rotations, etc.
	float timer = 0.0f;
	// Multiplier for speeding up (or slowing down) the global timer
	float timerSpeed = 0.25f;


	/** @brief Default depth stencil attachment used by the default render pass */
	struct {
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
	} depthStencil{};
};