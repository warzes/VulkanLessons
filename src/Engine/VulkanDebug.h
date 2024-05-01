#pragma once

namespace vks::debug
{
	void SetupDebuggingMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugUtilsMessengerCI);
	void SetupDebugging(VkInstance instance);
	void FreeDebugCallback(VkInstance instance);
}

// Wrapper for the VK_EXT_debug_utils extension
// The debug utils extensions allows us to put labels into command buffers and queues (to e.g. mark regions of interest) and to name Vulkan objects
namespace vks::debugutils
{
	void Setup(VkInstance instance);
	void CmdBeginLabel(VkCommandBuffer commandBuffer, const std::string& caption, const glm::vec4& color);
	void CmdEndLabel(VkCommandBuffer commandBuffer);
	void CmdInsertLabel(VkCommandBuffer commandBuffer, const std::string& labelName, const glm::vec4& color);

	// Functions for putting labels into a queue
	// Labels consist of a name and an optional color
	// How or if these are diplayed depends on the debugger used (RenderDoc e.g. displays both)
	void QueueBeginLabel(VkQueue queue, const std::string& labelName, const glm::vec4& color);
	void QueueInsertLabel(VkQueue queue, const std::string& labelName, const glm::vec4& color);
	void QueueEndLabel(VkQueue queue);

	// Function for naming Vulkan objects
	// In Vulkan, all objects (that can be named) are opaque unsigned 64 bit handles, and can be cased to uint64_t
	void SetObjectName(VkDevice device, VkObjectType objectType, uint64_t objectHandle, const std::string& objectName);
}