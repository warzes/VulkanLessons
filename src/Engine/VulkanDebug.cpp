#include "stdafx.h"
#include "VulkanDebug.h"
#include "Log.h"
#include "BaseFunc.h"
//-----------------------------------------------------------------------------
namespace vks::debug
{
	VkDebugUtilsMessengerEXT debugUtilsMessenger;
	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT{ nullptr };
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT{ nullptr };
}
//-----------------------------------------------------------------------------
namespace vks::debugutils
{
	PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT{ nullptr };
	PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT{ nullptr };
	PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT{ nullptr };

	PFN_vkQueueBeginDebugUtilsLabelEXT vkQueueBeginDebugUtilsLabelEXT{ nullptr };
	PFN_vkQueueInsertDebugUtilsLabelEXT vkQueueInsertDebugUtilsLabelEXT{ nullptr };
	PFN_vkQueueEndDebugUtilsLabelEXT vkQueueEndDebugUtilsLabelEXT{ nullptr };
	PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT{ nullptr };
}
//-----------------------------------------------------------------------------
VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessageCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* /*pUserData*/)
{
	// Select prefix depending on flags passed to the callback
	std::string prefix;

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) 
	{
		prefix = "VERBOSE: ";
#if defined(_WIN32)
		prefix = "\033[32m" + prefix + "\033[0m";
#endif
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) 
	{
		prefix = "INFO: ";
#if defined(_WIN32)
		prefix = "\033[36m" + prefix + "\033[0m";
#endif
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) 
	{
		prefix = "WARNING: ";
#if defined(_WIN32)
		prefix = "\033[33m" + prefix + "\033[0m";
#endif
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) 
	{
		prefix = "ERROR: ";
#if defined(_WIN32)
		prefix = "\033[31m" + prefix + "\033[0m";
#endif
	}

	// Display message to default output (console/logcat)
	std::stringstream debugMessage;
	debugMessage << prefix << "[" << pCallbackData->messageIdNumber << "][" << pCallbackData->pMessageIdName << "] : " << pCallbackData->pMessage;

	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		LogError(debugMessage.str());
	}
	else 
	{
		Fatal(debugMessage.str());
	}

	// The return value of this callback controls whether the Vulkan call that caused the validation message will be aborted or not
	// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message to abort
	// If you instead want to have calls abort, pass in VK_TRUE and the function will return VK_ERROR_VALIDATION_FAILED_EXT
	return VK_FALSE;
}
//-----------------------------------------------------------------------------
void vks::debug::SetupDebuggingMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugUtilsMessengerCI)
{
	debugUtilsMessengerCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugUtilsMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugUtilsMessengerCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	debugUtilsMessengerCI.pfnUserCallback = debugUtilsMessageCallback;
}
//-----------------------------------------------------------------------------
void vks::debug::SetupDebugging(VkInstance instance)
{
	vkCreateDebugUtilsMessengerEXT = FunctionCast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
	vkDestroyDebugUtilsMessengerEXT = FunctionCast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI{};
	SetupDebuggingMessengerCreateInfo(debugUtilsMessengerCI);
	VkResult result = vkCreateDebugUtilsMessengerEXT(instance, &debugUtilsMessengerCI, nullptr, &debugUtilsMessenger);
	assert(result == VK_SUCCESS);
}
//-----------------------------------------------------------------------------
void vks::debug::FreeDebugCallback(VkInstance instance)
{
	if (debugUtilsMessenger != VK_NULL_HANDLE)
		vkDestroyDebugUtilsMessengerEXT(instance, debugUtilsMessenger, nullptr);
}
//-----------------------------------------------------------------------------
void vks::debugutils::Setup(VkInstance instance)
{
	vkCmdBeginDebugUtilsLabelEXT = FunctionCast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT"));
	vkCmdEndDebugUtilsLabelEXT = FunctionCast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT"));
	vkCmdInsertDebugUtilsLabelEXT = FunctionCast<PFN_vkCmdInsertDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT"));

	vkQueueBeginDebugUtilsLabelEXT = FunctionCast<PFN_vkQueueBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkQueueBeginDebugUtilsLabelEXT"));
	vkQueueInsertDebugUtilsLabelEXT = FunctionCast<PFN_vkQueueInsertDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkQueueInsertDebugUtilsLabelEXT"));
	vkQueueEndDebugUtilsLabelEXT = FunctionCast<PFN_vkQueueEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkQueueEndDebugUtilsLabelEXT"));
	vkSetDebugUtilsObjectNameEXT = FunctionCast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT"));
}
//-----------------------------------------------------------------------------
void vks::debugutils::CmdBeginLabel(VkCommandBuffer commandBuffer, const std::string& caption, const glm::vec4& color)
{
	assert(vkCmdBeginDebugUtilsLabelEXT);
	if (!vkCmdBeginDebugUtilsLabelEXT) return;
	VkDebugUtilsLabelEXT labelInfo{ .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
	labelInfo.pLabelName = caption.c_str();
	memcpy(labelInfo.color, &color[0], sizeof(float) * 4);
	vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &labelInfo);
}
//-----------------------------------------------------------------------------
void vks::debugutils::CmdEndLabel(VkCommandBuffer commandBuffer)
{
	assert(vkCmdEndDebugUtilsLabelEXT);
	if (!vkCmdEndDebugUtilsLabelEXT) return;
	vkCmdEndDebugUtilsLabelEXT(commandBuffer);
}
//-----------------------------------------------------------------------------
void vks::debugutils::CmdInsertLabel(VkCommandBuffer commandBuffer, const std::string& labelName, const glm::vec4& color)
{
	assert(vkCmdInsertDebugUtilsLabelEXT);
	if (!vkCmdInsertDebugUtilsLabelEXT) return;
	VkDebugUtilsLabelEXT labelInfo = { .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
	labelInfo.pLabelName = labelName.c_str();
	memcpy(labelInfo.color, &color[0], sizeof(float) * 4);
	vkCmdInsertDebugUtilsLabelEXT(commandBuffer, &labelInfo);
}
//-----------------------------------------------------------------------------
void vks::debugutils::QueueBeginLabel(VkQueue queue, const std::string& labelName, const glm::vec4& color)
{
	assert(vkQueueBeginDebugUtilsLabelEXT);
	if (!vkQueueBeginDebugUtilsLabelEXT) return;
	VkDebugUtilsLabelEXT labelInfo = { .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
	labelInfo.pLabelName = labelName.c_str();
	memcpy(labelInfo.color, &color[0], sizeof(float) * 4);
	vkQueueBeginDebugUtilsLabelEXT(queue, &labelInfo);
}
//-----------------------------------------------------------------------------
void vks::debugutils::QueueInsertLabel(VkQueue queue, const std::string& labelName, const glm::vec4& color)
{
	assert(vkQueueInsertDebugUtilsLabelEXT);
	if (!vkQueueInsertDebugUtilsLabelEXT) return;
	VkDebugUtilsLabelEXT labelInfo = { .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
	labelInfo.pLabelName = labelName.c_str();
	memcpy(labelInfo.color, &color[0], sizeof(float) * 4);
	vkQueueInsertDebugUtilsLabelEXT(queue, &labelInfo);
}
//-----------------------------------------------------------------------------
void vks::debugutils::QueueEndLabel(VkQueue queue)
{
	assert(vkQueueEndDebugUtilsLabelEXT);
	if (!vkQueueEndDebugUtilsLabelEXT)  return;
	vkQueueEndDebugUtilsLabelEXT(queue);
}
//-----------------------------------------------------------------------------
void vks::debugutils::SetObjectName(VkDevice device, VkObjectType objectType, uint64_t objectHandle, const std::string& objectName)
{
	assert(vkSetDebugUtilsObjectNameEXT);
	if (!vkSetDebugUtilsObjectNameEXT) return;
	VkDebugUtilsObjectNameInfoEXT nameInfo = { .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
	nameInfo.objectType = objectType;
	nameInfo.objectHandle = objectHandle;
	nameInfo.pObjectName = objectName.c_str();
	vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
}
//-----------------------------------------------------------------------------