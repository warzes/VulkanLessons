#pragma once

class VulkanContext 
{
public:
	static VkQueue copyQueue;
	static VkQueue graphicsQueue;
	static VulkanDevice* device;
};

extern VulkanContext vulkanContext;