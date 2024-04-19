#pragma once

#include "VulkanTools.h"

struct SwapChainBuffer 
{
	VkImage image;
	VkImageView view;
};

class VulkanSwapChain
{
public:
	void Connect(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);



private:
	VkInstance instance;
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkSurfaceKHR surface;
public:
	VkFormat colorFormat;
	VkColorSpaceKHR colorSpace;
	VkSwapchainKHR swapChain = VK_NULL_HANDLE;
	uint32_t imageCount;
	std::vector<VkImage> images;
	std::vector<SwapChainBuffer> buffers;
	uint32_t queueNodeIndex = UINT32_MAX;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
	void initSurface(void* platformHandle, void* platformWindow);
#endif

	void create(uint32_t* width, uint32_t* height, bool vsync = false, bool fullscreen = false);
	VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex);
	VkResult queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
	void cleanup();
};