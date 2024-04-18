#pragma once

#include "VulkanSwapChain.h"
#include "VulkanDevice.h"
#include "VulkanUIOverlay.h"

struct RenderSystemCreateInfo final
{
	std::vector<const char*> enabledInstanceExtensions;
	bool validation = false;
	bool vsync = false;
};

struct EngineCreateInfo final
{
	struct
	{
		int width = 1024;
		int height = 768;
		const wchar_t* title = L"Game";
		bool fullscreen = false;
	} window;

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
	virtual void OnWindowResize();

	void Exit();
	float GetDeltaTime() const;

	//RenderSystem* GetRenderSystem() { return m_render; }
	std::string GetDeviceName() const;
	// This function is used to request a device memory type that supports all the property flags we request (e.g. device local, host visible)
	// Upon success it will return the index of the memory type that fits our requested memory properties
	// This is necessary as implementations can offer an arbitrary number of memory types with different
	// memory properties.
	// You can check https://vulkan.gpuinfo.org/ for details on different memory configurations
	uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties);


	VkDevice& GetDevice() { return device; }
	VulkanSwapChain& GetSwapChain() { return swapChain; }
	VkQueue& GetQueue() { return queue; }
	VkDescriptorPool& GetDescriptorPool() { return descriptorPool; }
	VkRenderPass& GetRenderPass() { return renderPass; }
	VkPipelineCache& GetPipelineCache() { return pipelineCache; }
	std::vector<VkFramebuffer>& GetFrameBuffers() { return frameBuffers; }

	bool& Prepared() { return prepared; }

private:
	bool create();
	void destroy();
	void frame();
	bool IsEnd() const;
	void setupDPIAwareness();
	void nextFrame();
	std::string getWindowTitle();
	void updateOverlay();
	void render();

	EngineAppImpl* m_data = nullptr;
	//RenderSystem* m_render = nullptr;

	// Frame counter to display fps
	uint32_t frameCounter = 0;
	uint32_t lastFPS = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp, tPrevEnd;
	/** @brief Last frame time measured using a high performance timer (if available) */
	float frameTimer = 1.0f;
	bool paused = false;
	// Defines a frame rate independent timer value clamped from -1.0...1.0
	// For use in animations, rotations, etc.
	float timer = 0.0f;
	// Multiplier for speeding up (or slowing down) the global timer
	float timerSpeed = 0.25f;

	/** @brief State of mouse/touch input */
	struct {
		struct {
			bool left = false;
			bool right = false;
			bool middle = false;
		} buttons;
		glm::vec2 position;
	} mouseState;

	bool initVulkan(const RenderSystemCreateInfo& createInfo);
	bool prepare(const RenderSystemCreateInfo& createInfo, void* hInstance, void* hwnd, uint32_t* width, uint32_t* height, bool fullscreen);
	void renderFinal();
	void renderDestroy();

	VkResult createInstance(bool enableValidation);
	void getEnabledFeatures() {} // TODO: если нужно
	void getEnabledExtensions() {} // TODO: если нужно

	void initSwapchain(void* hInstance, void* hwnd);
	void createCommandPool();
	void setupSwapChain(bool vsync, uint32_t* width, uint32_t* height, bool fullscreen);
	void createCommandBuffers();
	void createSynchronizationPrimitives();
	void setupDepthStencil(uint32_t width, uint32_t height);
	void setupRenderPass();
	void createPipelineCache();
	void setupFrameBuffer(uint32_t width, uint32_t height);

	void destroyCommandBuffers();

	/** @brief Loads a SPIR-V shader file for the given shader stage */
	VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage);

	void windowResize(uint32_t destWidth, uint32_t destHeight);

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

	/** @brief Encapsulated physical and logical vulkan device */
	vks::VulkanDevice* vulkanDevice;

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

	/** @brief Default depth stencil attachment used by the default render pass */
	struct {
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
	} depthStencil{};

	bool validation = false;
	bool prepared = false;
	bool resized = false;

	vks::UIOverlay UIOverlay;

	VkClearColorValue defaultClearColor = { { 0.025f, 0.025f, 0.025f, 1.0f } };

	bool m_vsync = false;
	bool m_fullscreen = false;
};