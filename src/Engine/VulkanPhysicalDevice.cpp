#include "stdafx.h"
#include "VulkanPhysicalDevice.h"
#include "Log.h"
//-----------------------------------------------------------------------------
struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};
//-----------------------------------------------------------------------------
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;
	// Logic to find queue family indices to populate struct with

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
			indices.graphicsFamily = i;
			indices.presentFamily = i;  // note: to do this properly, one should use vkGetPhysicalDeviceSurfaceSupportKHR and check surface support, but we don't have a surface
			// in general graphics queues are able to present. if the use has different needs, they should not use the default device.
		}

		/*VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport) {
			indices.presentFamily = i;
		}*/
		i++;
	}
	return indices;
}
//-----------------------------------------------------------------------------
VulkanPhysicalDevice::~VulkanPhysicalDevice()
{
	assert(!physicalDevice);
}
//-----------------------------------------------------------------------------
bool VulkanPhysicalDevice::Create(VkInstance vkInstance)
{
	uint32_t gpuDeviceCount = 0;
	// Get number of available physical devices
	VkResult result = vkEnumeratePhysicalDevices(vkInstance, &gpuDeviceCount, nullptr);
	if (result)
	{
		Fatal("vkEnumeratePhysicalDevices error : \n" + std::string(string_VkResult(result)));
		return false;
	}
	if (gpuDeviceCount == 0)
	{
		Fatal("No device with Vulkan support found");
		return false;
	}
	// Enumerate devices
	std::vector<VkPhysicalDevice> physicalDevices(gpuDeviceCount);
	result = vkEnumeratePhysicalDevices(vkInstance, &gpuDeviceCount, physicalDevices.data());
	if (result)
	{
		Fatal("Could not enumerate physical devices : \n" + std::string(string_VkResult(result)));
		return false;
	}

	// GPU selection

	VkPhysicalDevice selectDevice = VK_NULL_HANDLE;

	constexpr auto isDeviceSuitable = [](const VkPhysicalDevice device) -> bool {
		// look for all the features we want
		VkPhysicalDevicePushDescriptorPropertiesKHR devicePushDescriptors{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR,
			.pNext = nullptr
		};

		VkPhysicalDeviceProperties2 deviceProperties{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
			.pNext = &devicePushDescriptors,
		};
		vkGetPhysicalDeviceProperties2(device, &deviceProperties);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		auto queueFamilyData = findQueueFamilies(device);

		// right now we don't care so pick any gpu in the future implement a scoring system to pick the best device
		return deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && queueFamilyData.isComplete();
		};

	for (const auto& device : physicalDevices)
	{
		if (isDeviceSuitable(device))
		{
			selectDevice = device;
			break;
		}
	}
	if (selectDevice != VK_NULL_HANDLE)
	{
		physicalDevice = selectDevice;
	}
	else
	{
		LogWarning("failed to find a discrete GPU!");
		physicalDevice = physicalDevices[0];
	}

	// TODO: это дублируется в VulkanDevice
	// Store properties (including limits), features and memory properties of the physical device (so that examples can check against them)
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
	return true;
}
//-----------------------------------------------------------------------------
void VulkanPhysicalDevice::Destroy()
{
	physicalDevice = nullptr;
}
//-----------------------------------------------------------------------------
std::string VulkanPhysicalDevice::GetDeviceName() const
{
	return deviceProperties.deviceName;
}
//-----------------------------------------------------------------------------
uint32_t VulkanPhysicalDevice::GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties)
{
	// Iterate over all memory types available for the device used in this example
	for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}
		typeBits >>= 1;
	}

	throw "Could not find a suitable memory type!";
}
//-----------------------------------------------------------------------------