#include "stdafx.h"
#include "EngineApp.h"
#include "Log.h"
#include "KeyCodes.h"
//-----------------------------------------------------------------------------
#if defined(_WIN32)
constexpr const wchar_t* ClassName = L"EngineApp";
#endif // _WIN32
bool IsExit = false;
//-----------------------------------------------------------------------------
struct EngineAppImpl
{
#if defined(_WIN32)
	HINSTANCE hInstance = nullptr;
	HWND hwnd = nullptr;
	MSG msg{};
	uint32_t frameWidth = 0;
	uint32_t frameHeight = 0;
#endif // _WIN32

	float deltaTime = 0.01f;
};
//-----------------------------------------------------------------------------
EngineAppImpl* currentEngineAppImpl; // TODO: временно
EngineApp* currentEngineApp; // TODO: временно
//-----------------------------------------------------------------------------
#if defined(_WIN32)
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:
		IsExit = true;
		currentEngineApp->RenderPrepared() = false;
		PostQuitMessage(0);
		break;
	//case WM_DESTROY:
	//	PostQuitMessage(0);
	//	return 0;
	case WM_PAINT:
		ValidateRect(hwnd, nullptr);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case KEY_P:
			currentEngineApp->Paused() = !currentEngineApp->Paused();
			break;
		case KEY_F1:
			currentEngineApp->GetUIOverlay().visible = !currentEngineApp->GetUIOverlay().visible;
			currentEngineApp->GetUIOverlay().updated = true;
			break;
		case KEY_ESCAPE:
			IsExit = true;
			break;
		}
		currentEngineApp->OnKeyPressed((uint32_t)wParam);
		break;
	case WM_KEYUP:
		currentEngineApp->OnKeyUp((uint32_t)wParam);
		break;
	case WM_LBUTTONDOWN:
		currentEngineApp->mouseState.position = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
		currentEngineApp->mouseState.buttons.left = true;
		break;
	case WM_RBUTTONDOWN:
		currentEngineApp->mouseState.position = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
		currentEngineApp->mouseState.buttons.right = true;
		break;
	case WM_MBUTTONDOWN:
		currentEngineApp->mouseState.position = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
		currentEngineApp->mouseState.buttons.middle = true;
		break;
	case WM_LBUTTONUP:
		currentEngineApp->mouseState.buttons.left = false;
		break;
	case WM_RBUTTONUP:
		currentEngineApp->mouseState.buttons.right = false;
		break;
	case WM_MBUTTONUP:
		currentEngineApp->mouseState.buttons.middle = false;
		break;
	//case WM_MOUSEWHEEL:
	//{
	//	short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	//	break;
	//}
	case WM_MOUSEMOVE:
		currentEngineApp->handleMouseMove(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_SIZE:
		if ((currentEngineApp->RenderPrepared()) && (wParam != SIZE_MINIMIZED))
		{
			if ((currentEngineApp->resizing) || ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED)))
			{
				currentEngineApp->destWidth = LOWORD(lParam);
				currentEngineApp->destHeight = HIWORD(lParam);
				currentEngineApp->windowResize(currentEngineApp->destWidth, currentEngineApp->destHeight);
			}
		}
		break;
	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO minMaxInfo = (LPMINMAXINFO)lParam;
			minMaxInfo->ptMinTrackSize.x = 64;
			minMaxInfo->ptMinTrackSize.y = 64;
			break;
		}
	case WM_ENTERSIZEMOVE:
		currentEngineApp->resizing = true;
		break;
	case WM_EXITSIZEMOVE:
		currentEngineApp->resizing = false;
		break;
	default:
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
#endif // _WIN32
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
	m_data->hInstance = GetModuleHandle(nullptr);

	setupDPIAwareness();

	WNDCLASSEX wndClass{};
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.hInstance = m_data->hInstance;
	wndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wndClass.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);
	wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszClassName = ClassName;
	if (!RegisterClassEx(&wndClass))
	{
		Fatal("RegisterClassEx failed.");
		return false;
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (createInfo.window.fullscreen)
	{
		if ((createInfo.window.width != (uint32_t)screenWidth) && (createInfo.window.height != (uint32_t)screenHeight))
		{
			DEVMODE dmScreenSettings;
			memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
			dmScreenSettings.dmSize = sizeof(dmScreenSettings);
			dmScreenSettings.dmPelsWidth = createInfo.window.width;
			dmScreenSettings.dmPelsHeight = createInfo.window.height;
			dmScreenSettings.dmBitsPerPel = 32;
			dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			{
				if (MessageBox(NULL, L"Fullscreen Mode not supported!\n Switch to window mode?", L"Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				{
					createInfo.window.fullscreen = false;
				}
				else
				{
					return false;
				}
			}
			screenWidth = createInfo.window.width;
			screenHeight = createInfo.window.height;
		}
	}

	DWORD dwExStyle;
	DWORD dwStyle;

	if (createInfo.window.fullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}

	RECT windowRect;
	windowRect.left = 0L;
	windowRect.top = 0L;
	windowRect.right = createInfo.window.fullscreen ? (long)screenWidth : (long)createInfo.window.width;
	windowRect.bottom = createInfo.window.fullscreen ? (long)screenHeight : (long)createInfo.window.height;

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	m_data->hwnd = CreateWindowEx(0, ClassName, createInfo.window.title, dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
		nullptr, nullptr, m_data->hInstance, nullptr);

	if (!m_data->hwnd)
	{
		Fatal("Could not create window!");
		return false;
	}

	if (!createInfo.window.fullscreen)
	{
		// Center on screen
		uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
		uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
		SetWindowPos(m_data->hwnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	ShowWindow(m_data->hwnd, SW_SHOW);
	SetForegroundWindow(m_data->hwnd);
	SetFocus(m_data->hwnd);

	// TODO: вычислить реальный размер окна
	m_data->frameWidth = createInfo.window.width;
	m_data->frameHeight = createInfo.window.height;
	// TODO: также рендер может менять размер, проверить что там и как это использовать

	if (!initVulkan(createInfo.render) ||
		!prepareRender(createInfo.render, createInfo.window.fullscreen))
	{
		Fatal("RenderSystem create failed.");
		return false;
	}

	destWidth = m_data->frameWidth;
	destHeight = m_data->frameHeight;
	lastTimestamp = std::chrono::high_resolution_clock::now();
	tPrevEnd = lastTimestamp;
	IsExit = false;
	return true;
}
//-----------------------------------------------------------------------------
void EngineApp::setupDPIAwareness()
{
	typedef HRESULT* (__stdcall* SetProcessDpiAwarenessFunc)(PROCESS_DPI_AWARENESS);
	HMODULE shCore = LoadLibrary(L"Shcore.dll");
	if (shCore)
	{
		SetProcessDpiAwarenessFunc setProcessDpiAwareness = (SetProcessDpiAwarenessFunc)GetProcAddress(shCore, "SetProcessDpiAwareness");

		if (setProcessDpiAwareness != nullptr)
			setProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

		FreeLibrary(shCore);
	}
}
//-----------------------------------------------------------------------------
bool EngineApp::initVulkan(const RenderSystemCreateInfo& createInfo)
{
	validation = createInfo.validation;
#if defined(_DEBUG)
	validation = true;
#endif

	VkResult result;

	// Vulkan instance
	result = createInstance(validation);
	if (result)
	{
		vks::tools::exitFatal("Could not create Vulkan instance : \n" + vks::tools::errorString(result), result);
		return false;
	}

	// If requested, we enable the default validation layers for debugging
	if (validation)
		vks::debug::setupDebugging(instance);

	// Physical device
	uint32_t gpuCount = 0;
	// Get number of available physical devices
	VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr));
	if (gpuCount == 0)
	{
		vks::tools::exitFatal("No device with Vulkan support found", -1);
		return false;
	}
	// Enumerate devices
	std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
	result = vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());
	if (result)
	{
		vks::tools::exitFatal("Could not enumerate physical devices : \n" + vks::tools::errorString(result), result);
		return false;
	}

	// GPU selection

	// Select physical device to be used for the Vulkan example
	// Defaults to the first device unless specified by command line
	uint32_t selectedDevice = 0;

	// TODO: доделать выбор GPU

	physicalDevice = physicalDevices[selectedDevice];

	// Store properties (including limits), features and memory properties of the physical device (so that examples can check against them)
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

	// Derived examples can override this to set actual features (based on above readings) to enable for logical device creation
	getEnabledFeatures();

	// Vulkan device creation
	// This is handled by a separate class that gets a logical device representation and encapsulates functions related to a device
	vulkanDevice = new vks::VulkanDevice(physicalDevice);

	// Derived examples can enable extensions based on the list of supported extensions read from the physical device
	getEnabledExtensions();

	VkResult res = vulkanDevice->createLogicalDevice(enabledFeatures, enabledDeviceExtensions, deviceCreatepNextChain);
	if (res != VK_SUCCESS)
	{
		vks::tools::exitFatal("Could not create Vulkan device: \n" + vks::tools::errorString(res), res);
		return false;
	}
	device = vulkanDevice->logicalDevice;

	// Get a graphics queue from the device
	vkGetDeviceQueue(device, vulkanDevice->queueFamilyIndices.graphics, 0, &queue);

	// Find a suitable depth and/or stencil format
	VkBool32 validFormat{ false };
	// Samples that make use of stencil will require a depth + stencil format, so we select from a different list
	if (requiresStencil)
	{
		validFormat = vks::tools::getSupportedDepthStencilFormat(physicalDevice, &depthFormat);
	}
	else
	{
		validFormat = vks::tools::getSupportedDepthFormat(physicalDevice, &depthFormat);
	}
	assert(validFormat);

	swapChain.connect(instance, physicalDevice, device);

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
	appInfo.pApplicationName = "GameApp";
	appInfo.pEngineName = "VulkanEngine";
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
				supportedInstanceExtensions.push_back(extension.extensionName);
			}
		}
	}

	// Enabled requested instance extensions
	if (enabledInstanceExtensions.size() > 0)
	{
		for (const char* enabledExtension : enabledInstanceExtensions)
		{
			// Output message if requested extension is not available
			if (std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), enabledExtension) == supportedInstanceExtensions.end())
			{
				Fatal("Enabled instance extension '" + std::string(enabledExtension) + "' is not present at instance level");
			}
			instanceExtensions.push_back(enabledExtension);
		}
	}

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI{};
	if (enableValidation)
	{
		vks::debug::setupDebugingMessengerCreateInfo(debugUtilsMessengerCI);
		debugUtilsMessengerCI.pNext = instanceCreateInfo.pNext;
		instanceCreateInfo.pNext = &debugUtilsMessengerCI;
	}

	// Enable the debug utils extension if available (e.g. when debugging tools are present)
	if (enableValidation || std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != supportedInstanceExtensions.end())
	{
		instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	if (instanceExtensions.size() > 0)
	{
		instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	}

	// The VK_LAYER_KHRONOS_validation contains all current validation functionality.
	// Note that on Android this layer requires at least NDK r20
	const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
	if (enableValidation)
	{
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
		if (validationLayerPresent) {
			instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
			instanceCreateInfo.enabledLayerCount = 1;
		}
		else
		{
			Fatal("Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled");
		}
	}
	VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

	// If the debug utils extension is present we set up debug functions, so samples can label objects for debugging
	if (std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != supportedInstanceExtensions.end())
	{
		vks::debugutils::setup(instance);
	}

	return result;
}
//-----------------------------------------------------------------------------
bool EngineApp::prepareRender(const RenderSystemCreateInfo& createInfo, bool fullscreen)
{
	initSwapchain();
	createCommandPool();
	setupSwapChain(createInfo.vsync, &m_data->frameWidth, &m_data->frameHeight, fullscreen);
	createCommandBuffers();
	createSynchronizationPrimitives();
	setupDepthStencil(m_data->frameWidth, m_data->frameHeight);
	setupRenderPass();
	createPipelineCache();
	setupFrameBuffer(m_data->frameWidth, m_data->frameHeight);
	overlay = overlay && (!benchmark.active);
	if (overlay)
	{
		UIOverlay.device = vulkanDevice;
		UIOverlay.queue = queue;
		UIOverlay.shaders = {
			loadShader(getShaderBasePath() + "base/uioverlay.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
			loadShader(getShaderBasePath() + "base/uioverlay.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT),
		};
		UIOverlay.prepareResources();
		UIOverlay.preparePipeline(pipelineCache, renderPass, swapChain.colorFormat, depthFormat);
	}

	return true;
}
//-----------------------------------------------------------------------------
void EngineApp::initSwapchain()
{
	swapChain.initSurface(m_data->hInstance, m_data->hwnd);
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
	memAlloc.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
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
	while (PeekMessage(&m_data->msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&m_data->msg);
		DispatchMessage(&m_data->msg);
		if (m_data->msg.message == WM_QUIT)
		{
			IsExit = true;
			break;
		}
	}
	if (RenderPrepared() && !IsIconic(m_data->hwnd)) // TODO: может все таки обрабатывать события даже в свернутом окне?
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

	m_data->deltaTime = frameTimer = (float)tDiff / 1000.0f; // TODO: удалить frameTimer
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
#if defined(_WIN32)
		std::string windowTitle = getWindowTitle();
		SetWindowTextA(m_data->hwnd, windowTitle.c_str());
#endif
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
	OnFrame();
}
//-----------------------------------------------------------------------------
void EngineApp::windowResize(uint32_t destWidth, uint32_t destHeight)
{
	if (!prepared)
		return;

	prepared = false;
	resized = true;

	// Ensure all operations on the device have been finished before destroying resources
	vkDeviceWaitIdle(device);

	// Recreate swap chain
	m_data->frameWidth = destWidth;
	m_data->frameHeight = destHeight;
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

	OnWindowResize(destWidth, destHeight);

	prepared = true;
}
//-----------------------------------------------------------------------------
void EngineApp::handleMouseMove(int32_t x, int32_t y)
{
	int32_t dx = (int32_t)mouseState.position.x - x;
	int32_t dy = (int32_t)mouseState.position.y - y;

	bool handled = false;

	if (overlay) 
	{
		ImGuiIO& io = ImGui::GetIO();
		handled = io.WantCaptureMouse && UIOverlay.visible;
	}
	if (!handled)
		OnMouseMoved(x, y, dx, dy);

	mouseState.position = glm::vec2((float)x, (float)y);
}
//-----------------------------------------------------------------------------
void EngineApp::updateOverlay()
{
	if (!overlay)
		return;

	ImGuiIO& io = ImGui::GetIO();

	io.DisplaySize = ImVec2((float)m_data->frameWidth, (float)m_data->frameHeight);
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
	if (m_data->hwnd)
	{
		DestroyWindow(m_data->hwnd);
		m_data->hwnd = nullptr;
	}
	if (m_data->hInstance)
	{
		UnregisterClass(ClassName, m_data->hInstance);
		m_data->hInstance = nullptr;
	}
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

	delete vulkanDevice;

	if (validation)
		vks::debug::freeDebugCallback(instance);

	vkDestroyInstance(instance, nullptr);
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