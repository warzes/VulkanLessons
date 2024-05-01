#include "stdafx.h"
#include "VulkanInstance.h"
#include "VulkanDebug.h"
#include "Log.h"
//-----------------------------------------------------------------------------
VulkanInstance::~VulkanInstance()
{
	assert(!vkInstance);
}
//-----------------------------------------------------------------------------
bool VulkanInstance::Create(bool enableValidation, const std::vector<const char*>& enabledInstanceExtensions)
{
	enableValidationLayers = enableValidation;

	std::vector<const char*> validationLayers = {};
	if (enableValidationLayers)
	{
		validationLayers = checkValidationLayerSupport();
		if (validationLayers.size() == 0)
		{
			LogError("VulkanApp: no validation layers available");
			enableValidationLayers = false;
		}
	}

	std::vector<const char*> instanceExtensions = selectInstanceExtensions(enabledInstanceExtensions);

	VkApplicationInfo appInfo = { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.pApplicationName = "GameApp"; // TODO:
	appInfo.pEngineName = "VulkanEngine"; // TODO:
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // TODO:
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0); // TODO:
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo instanceCreateInfo = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instanceCreateInfo.pApplicationInfo = &appInfo;

	if (enableValidationLayers)
	{
		VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI{};
		vks::debug::SetupDebuggingMessengerCreateInfo(debugUtilsMessengerCI);
		debugUtilsMessengerCI.pNext = instanceCreateInfo.pNext;
		instanceCreateInfo.pNext = &debugUtilsMessengerCI;
	}

	if (instanceExtensions.size() > 0)
	{
		instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	}

	if (enableValidationLayers)
	{
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}

	VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance);
	if (result != VK_SUCCESS)
	{
		Fatal("Could not create Vulkan instance : \n" + std::string(string_VkResult(result)));
		return {};
	}

	// If requested, we enable the default validation layers for debugging
	if (enableValidationLayers)
		vks::debug::SetupDebugging(vkInstance);

	// If the debug utils extension is present we set up debug functions, so samples can label objects for debugging
	if (hasDebugUtilsExtension)
		vks::debugutils::Setup(vkInstance);

	return true;
}
//-----------------------------------------------------------------------------
void VulkanInstance::Destroy()
{
	if (enableValidationLayers)
		vks::debug::FreeDebugCallback(vkInstance);
	vkDestroyInstance(vkInstance, nullptr);
	vkInstance = nullptr;
}
//-----------------------------------------------------------------------------
PFN_vkVoidFunction VKAPI_CALL VulkanInstance::GetInstanceProcAddr(const char* pName)
{
	return vkGetInstanceProcAddr(vkInstance, pName);
}
//-----------------------------------------------------------------------------
std::vector<const char*> VulkanInstance::checkValidationLayerSupport() const
{
	// The VK_LAYER_KHRONOS_validation contains all current validation functionality.
	const std::vector<const char*> validationLayersKHR =
	{
		"VK_LAYER_KHRONOS_validation"
	};
	const std::vector<const char*> validationLayersLunarg =
	{
		"VK_LAYER_LUNARG_core_validation"
	};

	// Check if this layer is available at instance level
	uint32_t instanceLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(instanceLayerCount);
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, availableLayers.data());

	if (layerSupport(availableLayers, validationLayersKHR))
		return validationLayersKHR;
	if (layerSupport(availableLayers, validationLayersLunarg))
		return validationLayersLunarg;

	return {};
}
//-----------------------------------------------------------------------------
std::vector<const char*> VulkanInstance::selectInstanceExtensions(const std::vector<const char*>& enabledInstanceExtensions)
{
	std::vector<const char*> instanceExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
	#if defined(_WIN32)
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	#endif
	};

	auto supportedInstanceExtensionList = getInstanceExtensionsList();

	if (checkForExt(supportedInstanceExtensionList, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
	{
		instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		hasDeviceFeatures2 = true;
	}

	if (checkForExt(supportedInstanceExtensionList, VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME))
	{
		instanceExtensions.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
		hasSwapchainColorSpaceExtension = true;
	}

	if (enableValidationLayers)
	{
		// Enable the debug utils extension if available (e.g. when debugging tools are present)
		if (checkForExt(supportedInstanceExtensionList, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
		{
			instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			hasDebugUtilsExtension = true;
		}			
		if (checkForExt(supportedInstanceExtensionList, VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
		{
			instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			hasDebugReportExtension = true;
		}
	}

	// Enabled requested instance extensions
	if (enabledInstanceExtensions.size() > 0)
	{
		for (const char* enabledExtension : enabledInstanceExtensions)
		{
			if (checkForExt(supportedInstanceExtensionList, enabledExtension))
			{
				instanceExtensions.push_back(enabledExtension);
			}
			else
			{
				Fatal("Enabled instance extension '" + std::string(enabledExtension) + "' is not present at instance level");
			}
		}
	}

	return instanceExtensions;
}
//-----------------------------------------------------------------------------
bool VulkanInstance::layerSupport(const std::vector<VkLayerProperties>& sup, const std::vector<const char*>& dest) const
{
	for (auto& i : dest)
	{
		bool found = false;
		for (auto& r : sup)
		{
			if (std::strcmp(r.layerName, i) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
			return false;
	}
	return true;
}
//-----------------------------------------------------------------------------
std::vector<VkExtensionProperties> VulkanInstance::getInstanceExtensionsList() const
{
	// Get extensions supported by the instance and store for later use
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	if (extensionCount == 0)
		return {};

	std::vector<VkExtensionProperties> extensions(extensionCount);
	if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, &extensions.front()) != VK_SUCCESS)
		return {};

	return extensions;
}
//-----------------------------------------------------------------------------
bool VulkanInstance::checkForExt(const std::vector<VkExtensionProperties>& list, const char* name) const
{
	for (auto& r : list)
	{
		if (std::strcmp(name, r.extensionName) == 0)
			return true;
	}
	return false;
}
//-----------------------------------------------------------------------------