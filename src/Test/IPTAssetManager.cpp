#include "stdafx.h"
#include "IPTAssetManager.h"
#include "IPTVulkanContext.h"

void AssetManager::addModel(const std::string& filePath, const std::string& name, int modelLoadingFlags)
{
	auto it = models.find(name);
	if (it != models.end()) {
		// Asset already loaded
		return;
	}
	std::shared_ptr<ModelAsset> asset = std::make_shared<ModelAsset>();
	asset->model.loadFromFile(filePath, VulkanContext::device, VulkanContext::graphicsQueue, modelLoadingFlags);
	asset->filePath = filePath;
	asset->name = name;
	models.insert(std::make_pair(name, asset));
}

std::shared_ptr<ModelAsset> AssetManager::getAsset(const std::string& name)
{
	auto it = models.find(name);
	if (it != models.end()) {
		return it->second;
	}
	return nullptr;
}