#pragma once

#include "VulkanTools.h"

typedef struct _SwapChainBuffers {
	VkImage image;
	VkImageView view;
} SwapChainBuffer;

class VulkanSwapChain
{
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
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	void initSurface(ANativeWindow* window);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
	void initSurface(IDirectFB* dfb, IDirectFBSurface* window);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	void initSurface(wl_display* display, wl_surface* window);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	void initSurface(xcb_connection_t* connection, xcb_window_t window);
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
	void initSurface(void* view);
#elif (defined(_DIRECT2DISPLAY) || defined(VK_USE_PLATFORM_HEADLESS_EXT))
	void initSurface(uint32_t width, uint32_t height);
#if defined(_DIRECT2DISPLAY)
	void createDirect2DisplaySurface(uint32_t width, uint32_t height);
#endif
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
	void initSurface(screen_context_t screen_context, screen_window_t screen_window);
#endif
	void connect(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
	void create(uint32_t* width, uint32_t* height, bool vsync = false, bool fullscreen = false);
	VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex);
	VkResult queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
	void cleanup();
};
