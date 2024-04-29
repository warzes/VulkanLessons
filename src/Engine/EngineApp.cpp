#include "stdafx.h"
#include "EngineApp.h"
#include "Log.h"
#include "KeyCodes.h"
//-----------------------------------------------------------------------------
bool IsExit = false;
//-----------------------------------------------------------------------------
struct EngineAppImpl
{
	float deltaTime = 0.01f;
};
//-----------------------------------------------------------------------------
EngineAppImpl* currentEngineAppImpl; // TODO: ��������
EngineApp* currentEngineApp; // TODO: ��������
//-----------------------------------------------------------------------------
void EngineExit()
{
	IsExit = true;
	currentEngineApp->RenderPrepared() = false;
}
//-----------------------------------------------------------------------------
bool GetRenderPrepared()
{
	return currentEngineApp->RenderPrepared();
}
//-----------------------------------------------------------------------------
void EngineResize(uint32_t destWidth, uint32_t destHeight)
{
	currentEngineApp->resize(destWidth, destHeight);
}
//-----------------------------------------------------------------------------
EngineApp::EngineApp()
	: m_data(new EngineAppImpl)
{
	currentEngineAppImpl = m_data;
	currentEngineApp = this;
}
//-----------------------------------------------------------------------------
EngineApp::~EngineApp()
{
	currentEngineAppImpl = nullptr;
	currentEngineApp = nullptr;
	destroy();
	delete m_data;
}
//-----------------------------------------------------------------------------
void EngineApp::Run()
{
	if (create())
	{
		if (OnCreate())
		{
			RenderPrepared() = true;
			while (!isEnd())
			{
				frame();
			}
			RenderPrepared() = false;
			renderFinal();
			OnDestroy();
		}
	}
}
//-----------------------------------------------------------------------------
void EngineApp::Exit()
{
	IsExit = true;
}
//-----------------------------------------------------------------------------
float EngineApp::GetDeltaTime() const
{
	return m_data->deltaTime;
}
//-----------------------------------------------------------------------------
std::string EngineApp::GetDeviceName() const
{
	return deviceProperties.deviceName;
}
//-----------------------------------------------------------------------------
uint32_t EngineApp::GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties)
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
bool EngineApp::create()
{
	EngineCreateInfo createInfo = GetCreateInfo();

	if (!initPlatformApp(createInfo.window))
		return false;

	destWidth = GetFrameWidth();
	destHeight = GetFrameHeight();

	if (!initVulkan(createInfo.render) ||
		!prepareRender(createInfo.render, createInfo.window.fullscreen))
	{
		Fatal("RenderSystem create failed.");
		return false;
	}

	lastTimestamp = std::chrono::high_resolution_clock::now();
	tPrevEnd = lastTimestamp;
	IsExit = false;
	return true;
}
//-----------------------------------------------------------------------------
bool EngineApp::initVulkan(const RenderSystemCreateInfo& createInfo)
{
	validation = createInfo.validation;
#if defined(_DEBUG)
	validation = true;
#endif
	requiresStencil = createInfo.requiresStencil;

	setEnabledInstanceExtensions();

	// Vulkan instance
	 VkResult result = createInstance(validation);
	if (result)
	{
		Fatal("Could not create Vulkan instance : \n" + std::string(string_VkResult(result)));
		return false;
	}

	// If requested, we enable the default validation layers for debugging
	if (validation)
		vks::debug::setupDebugging(m_instance);

	if (!selectPhysicalDevice())
	{
		return false;
	}

	// TODO: ��� ����������� � VulkanDevice
	// Store properties (including limits), features and memory properties of the physical device (so that examples can check against them)
	vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperties);
	vkGetPhysicalDeviceFeatures(m_physicalDevice, &deviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &deviceMemoryProperties);

	// Derived examples can override this to set actual features (based on above readings) to enable for logical device creation
	getEnabledFeatures();

	// Vulkan device creation
	// This is handled by a separate class that gets a logical device representation and encapsulates functions related to a device
	m_vulkanDevice = new vks::VulkanDevice(m_physicalDevice);

	// Derived examples can enable extensions based on the list of supported extensions read from the physical device
	getEnabledExtensions();

	result = m_vulkanDevice->createLogicalDevice(enabledFeatures, enabledDeviceExtensions, deviceCreatepNextChain);
	if (result != VK_SUCCESS)
	{
		Fatal("Could not create Vulkan device: \n" + std::string(string_VkResult(result)));
		return false;
	}
	device = m_vulkanDevice->logicalDevice;

	// Get a graphics queue from the device
	vkGetDeviceQueue(device, m_vulkanDevice->queueFamilyIndices.graphics, 0, &queue);

	// Find a suitable depth and/or stencil format
	VkBool32 validFormat{ false };
	// Samples that make use of stencil will require a depth + stencil format, so we select from a different list
	if (requiresStencil)
	{
		validFormat = vks::tools::getSupportedDepthStencilFormat(m_physicalDevice, &depthFormat);
	}
	else
	{
		validFormat = vks::tools::getSupportedDepthFormat(m_physicalDevice, &depthFormat);
	}
	assert(validFormat);

	swapChain.Connect(m_instance, m_physicalDevice, device);

	// Create synchronization objects
	VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
	// Create a semaphore used to synchronize image presentation
	// Ensures that the image is displayed before we start submitting new commands to the queue
	VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.presentComplete));
	// Create a semaphore used to synchronize command submission
	// Ensures that the image is not presented until all commands have been submitted and executed
	VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.renderComplete));

	// Set up submit info structure
	// Semaphores will stay the same during application lifetime
	// Command buffer submission info is set by each example
	submitInfo = vks::initializers::submitInfo();
	submitInfo.pWaitDstStageMask = &submitPipelineStages;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &semaphores.presentComplete;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &semaphores.renderComplete;

	return true;
}
//-----------------------------------------------------------------------------
VkResult EngineApp::createInstance(bool enableValidation)
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "GameApp"; // TODO:
	appInfo.pEngineName = "VulkanEngine"; // TODO:
	appInfo.apiVersion = VK_API_VERSION_1_3;

	std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };
	// Enable surface extensions depending on os
#if defined(_WIN32)
	instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

	// Get extensions supported by the instance and store for later use
	uint32_t extCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
	if (extCount > 0)
	{
		std::vector<VkExtensionProperties> extensions(extCount);
		if (vkEnumerateInstanceExtensionProperties(nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
		{
			for (VkExtensionProperties& extension : extensions)
			{
				m_supportedInstanceExtensions.push_back(extension.extensionName);
			}
		}
	}

	// Enabled requested instance extensions
	if (enabledInstanceExtensions.size() > 0)
	{
		for (const char* enabledExtension : enabledInstanceExtensions)
		{
			// Output message if requested extension is not available
			if (std::find(m_supportedInstanceExtensions.begin(), m_supportedInstanceExtensions.end(), enabledExtension) == m_supportedInstanceExtensions.end())
			{
				Fatal("Enabled instance extension '" + std::string(enabledExtension) + "' is not present at instance level");
			}
			instanceExtensions.push_back(enabledExtension);
		}
	}

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = nullptr;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	if (enableValidation)
	{
		VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI{};
		vks::debug::setupDebugingMessengerCreateInfo(debugUtilsMessengerCI);
		debugUtilsMessengerCI.pNext = instanceCreateInfo.pNext;
		instanceCreateInfo.pNext = &debugUtilsMessengerCI;
	}

	// Enable the debug utils extension if available (e.g. when debugging tools are present)
	if (enableValidation || std::find(m_supportedInstanceExtensions.begin(), m_supportedInstanceExtensions.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != m_supportedInstanceExtensions.end())
	{
		instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	if (instanceExtensions.size() > 0)
	{
		instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	}

	if (enableValidation)
	{
		// The VK_LAYER_KHRONOS_validation contains all current validation functionality.
		const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
		// Check if this layer is available at instance level
		uint32_t instanceLayerCount;
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
		std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
		bool validationLayerPresent = false;
		for (VkLayerProperties& layer : instanceLayerProperties) {
			if (strcmp(layer.layerName, validationLayerName) == 0) {
				validationLayerPresent = true;
				break;
			}
		}
		if (validationLayerPresent)
		{
			instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
			instanceCreateInfo.enabledLayerCount = 1;
		}
		else
		{
			Fatal("Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled");
		}
	}
	VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);

	// If the debug utils extension is present we set up debug functions, so samples can label objects for debugging
	if (std::find(m_supportedInstanceExtensions.begin(), m_supportedInstanceExtensions.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != m_supportedInstanceExtensions.end())
	{
		vks::debugutils::setup(m_instance);
	}

	return result;
}
//-----------------------------------------------------------------------------
bool EngineApp::selectPhysicalDevice()
{
	uint32_t gpuDeviceCount = 0;
	// Get number of available physical devices
	VK_CHECK_RESULT(vkEnumeratePhysicalDevices(m_instance, &gpuDeviceCount, nullptr));
	if (gpuDeviceCount == 0)
	{
		Fatal("No device with Vulkan support found");
		return false;
	}
	// Enumerate devices
	std::vector<VkPhysicalDevice> physicalDevices(gpuDeviceCount);
	VkResult result = vkEnumeratePhysicalDevices(m_instance, &gpuDeviceCount, physicalDevices.data());
	if (result)
	{
		Fatal("Could not enumerate physical devices : \n" + std::string(string_VkResult(result)));
		return false;
	}

	// GPU selection

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

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

		auto queueFamilyData = vks::findQueueFamilies(device);

		// right now we don't care so pick any gpu in the future implement a scoring system to pick the best device
		return deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && queueFamilyData.isComplete();
		};

	for (const auto& device : physicalDevices) 
	{
		if (isDeviceSuitable(device)) 
		{
			physicalDevice = device;
			break;
		}
	}
	if (physicalDevice != VK_NULL_HANDLE)
	{
		m_physicalDevice = physicalDevice;
	}
	else
	{
		LogWarning("failed to find a discrete GPU!");
		m_physicalDevice = physicalDevices[0];
	}	

	return true;
}
//-----------------------------------------------------------------------------
bool EngineApp::prepareRender(const RenderSystemCreateInfo& createInfo, bool fullscreen)
{
	UIOverlay.subpass = createInfo.overlay.subpass;

	initSwapchain();
	createCommandPool();
	setupSwapChain(createInfo.vsync, &destWidth, &destHeight, fullscreen);
	createCommandBuffers();
	createSynchronizationPrimitives();
	setupDepthStencil(destWidth, destHeight);
	setupRenderPass();
	createPipelineCache();
	setupFrameBuffer(destWidth, destHeight);
	overlay = overlay && (!benchmark.active);
	if (overlay)
	{
		UIOverlay.device = m_vulkanDevice;
		UIOverlay.queue = queue;
		UIOverlay.shaders = {
			loadShader(getShadersPath() + "base/uioverlay.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
			loadShader(getShadersPath() + "base/uioverlay.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT),
		};
		UIOverlay.prepareResources();
		UIOverlay.preparePipeline(pipelineCache, renderPass, swapChain.colorFormat, depthFormat);
	}

	return true;
}
//-----------------------------------------------------------------------------
void EngineApp::initSwapchain()
{
	swapChain.initSurface(GetHINSTANCE(), GetHWND());
}
//-----------------------------------------------------------------------------
void EngineApp::createCommandPool()
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = swapChain.queueNodeIndex;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &cmdPool));
}
//-----------------------------------------------------------------------------
void EngineApp::setupSwapChain(bool vsync, uint32_t* width, uint32_t* height, bool fullscreen)
{
	m_vsync = vsync;
	m_fullscreen = fullscreen;
	swapChain.create(width, height, m_vsync, m_fullscreen);
}
//-----------------------------------------------------------------------------
void EngineApp::createCommandBuffers()
{
	// Create one command buffer for each swap chain image and reuse for rendering
	drawCmdBuffers.resize(swapChain.imageCount);

	VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		vks::initializers::commandBufferAllocateInfo(
			cmdPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			static_cast<uint32_t>(drawCmdBuffers.size()));

	VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, drawCmdBuffers.data()));
}
//-----------------------------------------------------------------------------
void EngineApp::createSynchronizationPrimitives()
{
	// Wait fences to sync command buffer access
	VkFenceCreateInfo fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	waitFences.resize(drawCmdBuffers.size());
	for (auto& fence : waitFences)
	{
		VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));
	}
}
//-----------------------------------------------------------------------------
void EngineApp::setupDepthStencil(uint32_t width, uint32_t height)
{
	// Create an optimal image used as the depth stencil attachment
	VkImageCreateInfo imageCI{};
	imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCI.imageType = VK_IMAGE_TYPE_2D;
	imageCI.format = depthFormat;
	imageCI.extent = { width, height, 1 };
	imageCI.mipLevels = 1;
	imageCI.arrayLayers = 1;
	imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VK_CHECK_RESULT(vkCreateImage(device, &imageCI, nullptr, &depthStencil.image));

	// Allocate memory for the image (device local) and bind it to our image
	VkMemoryAllocateInfo memAlloc{};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs{};
	vkGetImageMemoryRequirements(device, depthStencil.image, &memReqs);


	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = m_vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &depthStencil.memory));
	VK_CHECK_RESULT(vkBindImageMemory(device, depthStencil.image, depthStencil.memory, 0));

	// Create a view for the depth stencil image
	// Images aren't directly accessed in Vulkan, but rather through views described by a subresource range
	// This allows for multiple views of one image with differing ranges (e.g. for different layers)
	VkImageViewCreateInfo depthStencilViewCI{};
	depthStencilViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthStencilViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilViewCI.image = depthStencil.image;
	depthStencilViewCI.format = depthFormat;
	depthStencilViewCI.subresourceRange.baseMipLevel = 0;
	depthStencilViewCI.subresourceRange.levelCount = 1;
	depthStencilViewCI.subresourceRange.baseArrayLayer = 0;
	depthStencilViewCI.subresourceRange.layerCount = 1;
	depthStencilViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	// Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
	if (depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
		depthStencilViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	VK_CHECK_RESULT(vkCreateImageView(device, &depthStencilViewCI, nullptr, &depthStencil.view));
}
//-----------------------------------------------------------------------------
void EngineApp::setupRenderPass()
{
	std::array<VkAttachmentDescription, 2> attachments = {};
	// Color attachment
	attachments[0].format = swapChain.colorFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// Depth attachment
	attachments[1].format = depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;
	subpassDescription.pResolveAttachments = nullptr;

	// Subpass dependencies for layout transitions
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	dependencies[0].dependencyFlags = 0;

	dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].dstSubpass = 0;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].srcAccessMask = 0;
	dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	dependencies[1].dependencyFlags = 0;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
}
//-----------------------------------------------------------------------------
void EngineApp::createPipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	VK_CHECK_RESULT(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache));
}
//-----------------------------------------------------------------------------
void EngineApp::setupFrameBuffer(uint32_t width, uint32_t height)
{
	VkImageView attachments[2];

	// Depth/Stencil attachment is the same for all frame buffers
	attachments[1] = depthStencil.view;

	VkFramebufferCreateInfo frameBufferCreateInfo = {};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.pNext = NULL;
	frameBufferCreateInfo.renderPass = renderPass;
	frameBufferCreateInfo.attachmentCount = 2;
	frameBufferCreateInfo.pAttachments = attachments;
	frameBufferCreateInfo.width = width;
	frameBufferCreateInfo.height = height;
	frameBufferCreateInfo.layers = 1;

	// Create frame buffers for every swap chain image
	frameBuffers.resize(swapChain.imageCount);
	for (uint32_t i = 0; i < frameBuffers.size(); i++)
	{
		// Color attachment is the view of the swapchain image
		attachments[0] = swapChain.buffers[i].view;
		VK_CHECK_RESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &frameBuffers[i]));
	}
}
//-----------------------------------------------------------------------------
bool EngineApp::isEnd() const
{
	return IsExit;
}
//-----------------------------------------------------------------------------
void EngineApp::frame()
{
	if (!windowFrame())
	{
		IsExit = true;
		return;
	}

	if (RenderPrepared() && !isIconic()) // TODO: ����� ��� ���� ������������ ������� ���� � ��������� ����?
	{
		nextFrame();
	}
}
//-----------------------------------------------------------------------------
void EngineApp::nextFrame()
{
	auto tStart = std::chrono::high_resolution_clock::now();

	render();
	frameCounter++;
	auto tEnd = std::chrono::high_resolution_clock::now();
	auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

	m_data->deltaTime = frameTimer = (float)tDiff / 1000.0f; // TODO: ������� frameTimer
	OnUpdate(frameTimer);

	// Convert to clamped timer value
	if (!paused)
	{
		timer += timerSpeed * frameTimer;
		if (timer > 1.0)
		{
			timer -= 1.0f;
		}
	}
	float fpsTimer = (float)(std::chrono::duration<double, std::milli>(tEnd - lastTimestamp).count());
	if (fpsTimer > 1000.0f)
	{
		lastFPS = static_cast<uint32_t>((float)frameCounter * (1000.0f / fpsTimer));
		std::string windowTitle = getWindowTitle();
		SetTitle(windowTitle);

		frameCounter = 0;
		lastTimestamp = tEnd;
	}
	tPrevEnd = tEnd;

	// TODO: Cap UI overlay update rates
	updateOverlay();
}
//-----------------------------------------------------------------------------
void EngineApp::render()
{
	if (RenderPrepared())
		OnFrame();
}
//-----------------------------------------------------------------------------
void EngineApp::updateOverlay()
{
	if (!overlay)
		return;

	ImGuiIO& io = ImGui::GetIO();

	io.DisplaySize = ImVec2((float)destWidth, (float)destHeight);
	io.DeltaTime = frameTimer;

	io.MousePos = ImVec2(mouseState.position.x, mouseState.position.y);
	io.MouseDown[0] = mouseState.buttons.left && UIOverlay.visible;
	io.MouseDown[1] = mouseState.buttons.right && UIOverlay.visible;
	io.MouseDown[2] = mouseState.buttons.middle && UIOverlay.visible;

	ImGui::NewFrame();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::SetNextWindowPos(ImVec2(10 * UIOverlay.scale, 10 * UIOverlay.scale));
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Game", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGui::TextUnformatted("Game");
	ImGui::TextUnformatted(deviceProperties.deviceName);
	ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / lastFPS), lastFPS);

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 5.0f * UIOverlay.scale));
#endif
	ImGui::PushItemWidth(110.0f * UIOverlay.scale);
	OnUpdateUIOverlay(&UIOverlay);
	ImGui::PopItemWidth();
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	ImGui::PopStyleVar();
#endif

	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::Render();

	if (UIOverlay.update() || UIOverlay.updated) 
	{
		buildCommandBuffers();
		UIOverlay.updated = false;
	}
}
//-----------------------------------------------------------------------------
void EngineApp::renderFinal()
{
	// Flush device to make sure all resources can be freed
	if (device != VK_NULL_HANDLE)
	{
		vkDeviceWaitIdle(device);
	}
}
//-----------------------------------------------------------------------------
void EngineApp::destroy()
{
	renderDestroy();
	closePlatformApp();
}
//-----------------------------------------------------------------------------
void EngineApp::renderDestroy()
{
	// Clean up Vulkan resources
	swapChain.cleanup();
	if (descriptorPool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	destroyCommandBuffers();
	if (renderPass != VK_NULL_HANDLE)
		vkDestroyRenderPass(device, renderPass, nullptr);

	for (uint32_t i = 0; i < frameBuffers.size(); i++)
		vkDestroyFramebuffer(device, frameBuffers[i], nullptr);

	for (auto& shaderModule : shaderModules)
		vkDestroyShaderModule(device, shaderModule, nullptr);

	vkDestroyImageView(device, depthStencil.view, nullptr);
	vkDestroyImage(device, depthStencil.image, nullptr);
	vkFreeMemory(device, depthStencil.memory, nullptr);

	vkDestroyPipelineCache(device, pipelineCache, nullptr);

	vkDestroyCommandPool(device, cmdPool, nullptr);

	vkDestroySemaphore(device, semaphores.presentComplete, nullptr);
	vkDestroySemaphore(device, semaphores.renderComplete, nullptr);
	for (auto& fence : waitFences)
		vkDestroyFence(device, fence, nullptr);

	if (overlay)
		UIOverlay.freeResources();

	delete m_vulkanDevice;

	if (validation)
		vks::debug::freeDebugCallback(m_instance);

	vkDestroyInstance(m_instance, nullptr);
}
//-----------------------------------------------------------------------------
void EngineApp::destroyCommandBuffers()
{
	vkFreeCommandBuffers(device, cmdPool, static_cast<uint32_t>(drawCmdBuffers.size()), drawCmdBuffers.data());
}
//-----------------------------------------------------------------------------
std::string EngineApp::getWindowTitle()
{
	std::string device(GetDeviceName());
	std::string windowTitle;
	windowTitle = "Game - " + device;
	windowTitle += " - " + std::to_string(frameCounter) + " fps";
	return windowTitle;
}
//-----------------------------------------------------------------------------
VkPipelineShaderStageCreateInfo EngineApp::loadShader(std::string fileName, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	shaderStage.module = vks::tools::loadShader(androidApp->activity->assetManager, fileName.c_str(), device);
#else
	shaderStage.module = vks::tools::loadShader(fileName.c_str(), device);
#endif
	shaderStage.pName = "main";
	assert(shaderStage.module != VK_NULL_HANDLE);
	shaderModules.push_back(shaderStage.module);
	return shaderStage;
}
//-----------------------------------------------------------------------------
void EngineApp::resize(uint32_t destWidth, uint32_t destHeight)
{
	prepared = false;
	resized = true;
	this->destWidth = destWidth;
	this->destHeight = destHeight;

	// Ensure all operations on the device have been finished before destroying resources
	vkDeviceWaitIdle(device);

	// Recreate swap chain
	setupSwapChain(m_vsync, &destWidth, &destHeight, m_fullscreen);

	// Recreate the frame buffers
	vkDestroyImageView(device, depthStencil.view, nullptr);
	vkDestroyImage(device, depthStencil.image, nullptr);
	vkFreeMemory(device, depthStencil.memory, nullptr);
	setupDepthStencil(destWidth, destHeight);
	for (uint32_t i = 0; i < frameBuffers.size(); i++)
		vkDestroyFramebuffer(device, frameBuffers[i], nullptr);

	setupFrameBuffer(destWidth, destHeight);

	if ((destWidth > 0.0f) && (destHeight > 0.0f))
	{
		if (overlay)
		{
			UIOverlay.resize(destWidth, destHeight);
		}
	}

	// Command buffers need to be recreated as they may store references to the recreated frame buffer
	destroyCommandBuffers();
	createCommandBuffers();
	buildCommandBuffers();

	// SRS - Recreate fences in case number of swapchain images has changed on resize
	for (auto& fence : waitFences)
		vkDestroyFence(device, fence, nullptr);

	createSynchronizationPrimitives();

	vkDeviceWaitIdle(device);

	prepared = true;
}
//-----------------------------------------------------------------------------
void EngineApp::drawUI(const VkCommandBuffer commandBuffer)
{
	if (overlay && UIOverlay.visible)
	{
		const VkViewport viewport = vks::initializers::viewport((float)destWidth, (float)destHeight, 0.0f, 1.0f);
		const VkRect2D scissor = vks::initializers::rect2D(destWidth, destHeight, 0, 0);
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		UIOverlay.draw(commandBuffer);
	}
}
//-----------------------------------------------------------------------------
void EngineApp::prepareFrame()
{
	// Acquire the next image from the swap chain
	VkResult result = swapChain.acquireNextImage(semaphores.presentComplete, &currentBuffer);
	// Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE)
	// SRS - If no longer optimal (VK_SUBOPTIMAL_KHR), wait until submitFrame() in case number of swapchain images will change on resize
	if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) 
	{
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			windowResize(destWidth, destHeight);
		}
		return;
	}
	else {
		VK_CHECK_RESULT(result);
	}
}
//-----------------------------------------------------------------------------
void EngineApp::submitFrame()
{
	VkResult result = swapChain.queuePresent(queue, currentBuffer, semaphores.renderComplete);
	// Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
	if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
		windowResize(destWidth, destHeight);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			return;
		}
	}
	else {
		VK_CHECK_RESULT(result);
	}
	VK_CHECK_RESULT(vkQueueWaitIdle(queue));
}
//-----------------------------------------------------------------------------
void EngineApp::renderFrame()
{
	EngineApp::prepareFrame();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
	VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
	EngineApp::submitFrame();
}
//-----------------------------------------------------------------------------