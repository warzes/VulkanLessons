#pragma once

/*
Descriptor indexing (VK_EXT_descriptor_indexing)
Demonstrates the use of VK_EXT_descriptor_indexing for creating descriptor sets with a variable size that can be dynamically indexed in a shader using GL_EXT_nonuniform_qualifier and SPV_EXT_descriptor_indexing
*/

class DescriptorIndexingApp final : public EngineApp
{
public:
	EngineCreateInfo GetCreateInfo() const final { return {}; }
	bool OnCreate() final;
	void OnDestroy() final;
	void OnUpdate(float deltaTime) final;
	void OnFrame() final;
	void OnUpdateUIOverlay(vks::UIOverlay* overlay) final;

	void OnWindowResize(uint32_t destWidth, uint32_t destHeight) final;
	void OnKeyPressed(uint32_t key) final;
	void OnKeyUp(uint32_t key) final;
	void OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy) final;

private:
	Camera camera;
};