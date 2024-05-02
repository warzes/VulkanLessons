#include "stdafx.h"
#include "IPTVulkanContext.h"

VulkanContext vulkanContext{};

VkQueue VulkanContext::copyQueue = VK_NULL_HANDLE;
VkQueue VulkanContext::graphicsQueue = VK_NULL_HANDLE;
VulkanDevice* VulkanContext::device = nullptr;