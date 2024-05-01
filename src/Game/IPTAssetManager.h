#pragma once

class Asset
{
public:
	std::string name;
	std::string filePath;
	virtual ~Asset() {};
};

class TextureAsset : public Asset
{
};

class ModelAsset : public Asset
{
public:
	vkglTF::Model model;
};

class AssetManager
{
private:
	std::map<std::string, std::shared_ptr<ModelAsset>> models;
public:
	void addModel(const std::string& filePath, const std::string& name, int modelLoadingFlags = 0);
	std::shared_ptr<ModelAsset> getAsset(const std::string& name);
};