#pragma once

/*
Variable rate shading (VK_KHR_fragment_shading_rate)
Uses a special image that contains variable shading rates to vary the number of fragment shader invocations across the framebuffer. This makes it possible to lower fragment shader invocations for less important/less noisy parts of the framebuffer.
*/

class VariableRateShadingApp final : public EngineApp
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