#pragma once

class VulkanInstance final
{
public:
	VkInstance Create(bool enableValidationLayers, const std::vector<const char*>& enabledInstanceExtensions);

	bool hasDeviceFeatures2 = false;
	bool hasDebugUtilsExtension = false;
	bool hasDebugReportExtension = false;

private:
	std::vector<const char*> checkValidationLayerSupport() const;
	std::vector<const char*> selectInstanceExtensions(bool enableValidationLayers, const std::vector<const char*>& enabledInstanceExtensions);
	bool layerSupport(const std::vector<VkLayerProperties>& sup, const std::vector<const char*>& dest) const;
	std::vector<VkExtensionProperties> getInstanceExtensionsList() const;
	bool checkForExt(const std::vector<VkExtensionProperties>& list, const char* name) const;
};