#pragma once

/*
Conditional rendering (VK_EXT_conditional_rendering)
Demonstrates the use of VK_EXT_conditional_rendering to conditionally dispatch render commands based on values from a dedicated buffer. This allows e.g. visibility toggles without having to rebuild command buffers (blog post).
https://www.saschawillems.de/tutorials/vulkan/conditional_rendering
*/

class ConditionalRenderApp final : public EngineApp
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