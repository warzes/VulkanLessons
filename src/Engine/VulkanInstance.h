#pragma once

class VulkanInstance final
{
public:
	~VulkanInstance();

	bool Create(bool enableValidationLayers, const std::vector<const char*>& enabledInstanceExtensions);
	void Destroy();

	PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(const char* pName);

	VkInstance vkInstance{ VK_NULL_HANDLE }; // Vulkan instance, stores all per-application states
	bool validationLayers = false;

	bool hasDeviceFeatures2 = false;
	bool hasDebugUtilsExtension = false;
	bool hasDebugReportExtension = false;
	bool hasSwapchainColorSpaceExtension = false;

private:
	std::vector<const char*> checkValidationLayerSupport() const;
	std::vector<const char*> selectInstanceExtensions(bool enableValidationLayers, const std::vector<const char*>& enabledInstanceExtensions);
	bool layerSupport(const std::vector<VkLayerProperties>& sup, const std::vector<const char*>& dest) const;
	std::vector<VkExtensionProperties> getInstanceExtensionsList() const;
	bool checkForExt(const std::vector<VkExtensionProperties>& list, const char* name) const;
};