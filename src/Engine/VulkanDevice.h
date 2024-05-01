#pragma once

#include "VulkanBuffer.h"

class VulkanDevice final
{
public:
	explicit VulkanDevice(VkPhysicalDevice physicalDevice);
	~VulkanDevice();

	/**
	* Create the logical device based on the assigned physical device, also gets default queue family indices
	*
	* @param enabledFeatures Can be used to enable certain features upon device creation
	* @param pNextChain Optional chain of pointer to extension structures
	* @param useSwapChain Set to false for headless rendering to omit the swapchain device extensions
	* @param requestedQueueTypes Bit flags specifying the queue types to be requested from the device
	*
	* @return VkResult of the device creation call
	*/
	VkResult CreateLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions, void* pNextChain, bool useSwapChain = true, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
	/**
	* Create a command pool for allocation command buffers from
	*
	* @param queueFamilyIndex Family index of the queue to create the command pool for
	* @param createFlags (Optional) Command pool creation flags (Defaults to VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
	*
	* @note Command buffers allocated from the created pool can only be submitted to a queue with the same family index
	*
	* @return A handle to the created command buffer
	*/
	VkCommandPool CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties properties{};
	VkPhysicalDeviceFeatures features{};
	VkPhysicalDeviceFeatures enabledFeatures{};
	VkPhysicalDeviceMemoryProperties memoryProperties{};
	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	std::vector<std::string> supportedExtensions;

	// Contains queue family indices
	struct
	{
		uint32_t graphics = 0;
		uint32_t compute = 0;
		uint32_t transfer = 0;
	} queueFamilyIndices;

	VkDevice logicalDevice = VK_NULL_HANDLE;
	VkCommandPool commandPool = VK_NULL_HANDLE;

	uint32_t        getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr) const;
	uint32_t        getQueueFamilyIndex(VkQueueFlags queueFlags) const;

	VkResult        createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data = nullptr);
	VkResult        createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VulkanBuffer* buffer, VkDeviceSize size, void* data = nullptr);
	void            copyBuffer(VulkanBuffer* src, VulkanBuffer* dst, VkQueue queue, VkBufferCopy* copyRegion = nullptr);

	VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin = false);
	VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false);
	void            flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free = true);
	void            flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
	bool            extensionSupported(std::string extension);
	VkFormat        getSupportedDepthFormat(bool checkSamplingSupport);
};