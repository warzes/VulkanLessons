#pragma once

namespace vks
{
	namespace debug
	{
		// Load debug function pointers and set debug callback
		void setupDebugging(VkInstance instance);
		// Clear debug callback
		void freeDebugCallback(VkInstance instance);
		// Used to populate a VkDebugUtilsMessengerCreateInfoEXT with our example messenger function and desired flags
		void setupDebugingMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugUtilsMessengerCI);
	}

	// Wrapper for the VK_EXT_debug_utils extension
	// These can be used to name Vulkan objects for debugging tools like RenderDoc
	namespace debugutils
	{
		void setup(VkInstance instance);
		void cmdBeginLabel(VkCommandBuffer cmdbuffer, std::string caption, glm::vec4 color);
		void cmdEndLabel(VkCommandBuffer cmdbuffer);
	}
}