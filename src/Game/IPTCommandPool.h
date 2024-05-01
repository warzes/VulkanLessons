#pragma once

class CommandPool {
private:
	VkDevice device;
	uint32_t queueFamilyIndex;
	VkCommandPoolCreateFlags flags;
public:
	VkCommandPool handle;
	CommandPool(VkDevice device) {
		this->device = device;
	}
	~CommandPool() {
		// @todo
	}
	void create() {
		VkCommandPoolCreateInfo CI{};
		CI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		CI.queueFamilyIndex = queueFamilyIndex;
		CI.flags = flags;
		VK_CHECK_RESULT(vkCreateCommandPool(device, &CI, nullptr, &handle));
	}
	void setQueueFamilyIndex(uint32_t queueFamilyIndex) {
		this->queueFamilyIndex = queueFamilyIndex;
	}
	void setFlags(VkCommandPoolCreateFlags flags) {
		this->flags = flags;
	}
};