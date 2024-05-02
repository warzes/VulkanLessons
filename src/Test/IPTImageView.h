#pragma once

#include "IPTImage.h"

class ImageView {
private:
	VulkanDevice* device = nullptr;
	Image* image = nullptr;
	VkImageViewType type;
	VkFormat format;
	VkImageSubresourceRange range;
public:
	VkImageView handle;
	ImageView(VulkanDevice* device) {
		this->device = device;
	}
	~ImageView() {
		// @todo
	}
	void create() {
		VkImageViewCreateInfo CI = vks::initializers::imageViewCreateInfo();
		CI.viewType = type;
		CI.format = format;
		CI.subresourceRange = range;
		CI.image = image->handle;
		VK_CHECK_RESULT(vkCreateImageView(device->logicalDevice, &CI, nullptr, &handle));
	}
	void setImage(Image* image) {
		this->image = image;
	}
	void setType(VkImageViewType type) {
		this->type = type;
	}
	void setFormat(VkFormat format) {
		this->format = format;
	}
	void setSubResourceRange(VkImageSubresourceRange range) {
		this->range = range;
	}
};