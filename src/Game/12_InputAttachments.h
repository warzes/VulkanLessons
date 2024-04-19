#pragma once

/*
Uses input attachments to read framebuffer contents from a previous sub pass at the same pixel position within a single render pass. This can be used for basic post processing or image composition (blog entry).
https://www.saschawillems.de/blog/2018/07/19/vulkan-input-attachments-and-sub-passes/
*/

class InputAttachmentsApp final : public EngineApp
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