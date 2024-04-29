#include "stdafx.h"
#include "100_texturesparseresidency.h"
//-----------------------------------------------------------------------------
bool TextureSparseResidencyApp::OnCreate()
{
	std::cout.imbue(std::locale(""));
	camera.type = Camera::CameraType::lookat;
	camera.setPosition(glm::vec3(0.0f, 0.0f, -12.0f));
	camera.setRotation(glm::vec3(-90.0f, 0.0f, 0.0f));
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 256.0f);

	// Check if the GPU supports sparse residency for 2D images
	if (!m_vulkanDevice->features.sparseResidencyImage2D) {
		vks::tools::exitFatal("Device does not support sparse residency for 2D images!", VK_ERROR_FEATURE_NOT_PRESENT);
	}
	loadAssets();
	prepareUniformBuffers();
	// Create a virtual texture with max. possible dimension (does not take up any VRAM yet)
	prepareSparseTexture(4096, 4096, 1, VK_FORMAT_R8G8B8A8_UNORM);
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void TextureSparseResidencyApp::OnDestroy()
{
	// Clean up used Vulkan resources
	// Note : Inherited destructor cleans up resources stored in base class
	destroyTextureImage(texture);
	vkDestroySemaphore(device, bindSparseSemaphore, nullptr);
	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	uniformBuffer.destroy();
}
//-----------------------------------------------------------------------------
void TextureSparseResidencyApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void TextureSparseResidencyApp::OnFrame()
{
	updateUniformBuffers();
	draw();
}
//-----------------------------------------------------------------------------
void TextureSparseResidencyApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		if (overlay->sliderFloat("LOD bias", &uniformData.lodBias, -(float)texture.mipLevels, (float)texture.mipLevels)) {
			updateUniformBuffers();
		}
		if (overlay->button("Fill random pages")) {
			fillRandomPages();
		}
		if (overlay->button("Flush random pages")) {
			flushRandomPages();
		}
		if (overlay->button("Fill mip tail")) {
			fillMipTail();
		}
	}
	if (overlay->header("Statistics")) {
		uint32_t respages = 0;
		std::for_each(texture.pages.begin(), texture.pages.end(), [&respages](VirtualTexturePage page) { respages += (page.resident()) ? 1 : 0; });
		overlay->text("Resident pages: %d of %d", respages, static_cast<uint32_t>(texture.pages.size()));
		overlay->text("Mip tail starts at: %d", texture.mipTailStart);
	}
}
//-----------------------------------------------------------------------------
void TextureSparseResidencyApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void TextureSparseResidencyApp::OnKeyPressed(uint32_t key)
{
	if (key == KEY_F2)
	{
		if (camera.type == Camera::CameraType::lookat)
		{
			camera.type = Camera::CameraType::firstperson;
		}
		else
		{
			camera.type = Camera::CameraType::lookat;
		}
	}

	if (camera.type == Camera::firstperson)
	{
		switch (key)
		{
		case KEY_W:
			camera.keys.up = true;
			break;
		case KEY_S:
			camera.keys.down = true;
			break;
		case KEY_A:
			camera.keys.left = true;
			break;
		case KEY_D:
			camera.keys.right = true;
			break;
		}
	}
}
//-----------------------------------------------------------------------------
void TextureSparseResidencyApp::OnKeyUp(uint32_t key)
{
	if (camera.type == Camera::firstperson)
	{
		switch (key)
		{
		case KEY_W:
			camera.keys.up = false;
			break;
		case KEY_S:
			camera.keys.down = false;
			break;
		case KEY_A:
			camera.keys.left = false;
			break;
		case KEY_D:
			camera.keys.right = false;
			break;
		}
	}
}
//-----------------------------------------------------------------------------
void TextureSparseResidencyApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
{
	if (mouseState.buttons.left)
	{
		camera.rotate(glm::vec3(dy * camera.rotationSpeed, -dx * camera.rotationSpeed, 0.0f));
	}
	if (mouseState.buttons.right)
	{
		camera.translate(glm::vec3(-0.0f, 0.0f, dy * .005f));
	}
	if (mouseState.buttons.middle)
	{
		camera.translate(glm::vec3(-dx * 0.005f, -dy * 0.005f, 0.0f));
	}
}
//-----------------------------------------------------------------------------

/*
	Virtual texture page
	Contains all functions and objects for a single page of a virtual texture
 */

VirtualTexturePage::VirtualTexturePage()
{
	// Pages are initially not backed up by memory (non-resident)
	imageMemoryBind.memory = VK_NULL_HANDLE;
}

bool VirtualTexturePage::resident()
{
	return (imageMemoryBind.memory != VK_NULL_HANDLE);
}

// Allocate Vulkan memory for the virtual page
bool VirtualTexturePage::allocate(VkDevice device, uint32_t memoryTypeIndex)
{
	if (imageMemoryBind.memory != VK_NULL_HANDLE)
	{
		return false;
	};

	imageMemoryBind = {};

	VkMemoryAllocateInfo allocInfo = vks::initializers::memoryAllocateInfo();
	allocInfo.allocationSize = size;
	allocInfo.memoryTypeIndex = memoryTypeIndex;
	VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &imageMemoryBind.memory));

	VkImageSubresource subResource{};
	subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subResource.mipLevel = mipLevel;
	subResource.arrayLayer = layer;

	// Sparse image memory binding
	imageMemoryBind.subresource = subResource;
	imageMemoryBind.extent = extent;
	imageMemoryBind.offset = offset;
	return true;
}

// Release Vulkan memory allocated for this page
bool VirtualTexturePage::release(VkDevice device)
{
	del = false;
	if (imageMemoryBind.memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(device, imageMemoryBind.memory, nullptr);
		imageMemoryBind.memory = VK_NULL_HANDLE;
		return true;
	}
	return false;
}

/*
	Virtual texture
	Contains the virtual pages and memory binding information for a whole virtual texture
 */

VirtualTexturePage* VirtualTexture::addPage(VkOffset3D offset, VkExtent3D extent, const VkDeviceSize size, const uint32_t mipLevel, uint32_t layer)
{
	VirtualTexturePage newPage{};
	newPage.offset = offset;
	newPage.extent = extent;
	newPage.size = size;
	newPage.mipLevel = mipLevel;
	newPage.layer = layer;
	newPage.index = static_cast<uint32_t>(pages.size());
	newPage.imageMemoryBind = {};
	newPage.imageMemoryBind.offset = offset;
	newPage.imageMemoryBind.extent = extent;
	newPage.del = false;
	pages.push_back(newPage);
	return &pages.back();
}

// Call before sparse binding to update memory bind list etc.
void VirtualTexture::updateSparseBindInfo(std::vector<VirtualTexturePage>& bindingChangedPages, bool del)
{
	// Update list of memory-backed sparse image memory binds
	//sparseImageMemoryBinds.resize(pages.size());
	sparseImageMemoryBinds.clear();
	for (auto page : bindingChangedPages)
	{
		sparseImageMemoryBinds.push_back(page.imageMemoryBind);
		if (del)
		{
			sparseImageMemoryBinds[sparseImageMemoryBinds.size() - 1].memory = VK_NULL_HANDLE;
		}
	}
	// Update sparse bind info
	bindSparseInfo = vks::initializers::bindSparseInfo();
	// todo: Semaphore for queue submission
	// bindSparseInfo.signalSemaphoreCount = 1;
	// bindSparseInfo.pSignalSemaphores = &bindSparseSemaphore;

	// Image memory binds
	imageMemoryBindInfo = {};
	imageMemoryBindInfo.image = image;
	imageMemoryBindInfo.bindCount = static_cast<uint32_t>(sparseImageMemoryBinds.size());
	imageMemoryBindInfo.pBinds = sparseImageMemoryBinds.data();
	bindSparseInfo.imageBindCount = (imageMemoryBindInfo.bindCount > 0) ? 1 : 0;
	bindSparseInfo.pImageBinds = &imageMemoryBindInfo;

	// Opaque image memory binds for the mip tail
	opaqueMemoryBindInfo.image = image;
	opaqueMemoryBindInfo.bindCount = static_cast<uint32_t>(opaqueMemoryBinds.size());
	opaqueMemoryBindInfo.pBinds = opaqueMemoryBinds.data();
	bindSparseInfo.imageOpaqueBindCount = (opaqueMemoryBindInfo.bindCount > 0) ? 1 : 0;
	bindSparseInfo.pImageOpaqueBinds = &opaqueMemoryBindInfo;
}

// Release all Vulkan resources
void VirtualTexture::destroy()
{
	for (auto page : pages)
	{
		page.release(device);
	}
	for (auto bind : opaqueMemoryBinds)
	{
		vkFreeMemory(device, bind.memory, nullptr);
	}
	// Clean up mip tail
	if (mipTailimageMemoryBind.memory != VK_NULL_HANDLE) {
		vkFreeMemory(device, mipTailimageMemoryBind.memory, nullptr);
	}
}