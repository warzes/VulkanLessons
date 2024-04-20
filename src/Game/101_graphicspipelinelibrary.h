#pragma once

/*
Graphics pipeline library (VK_EXT_graphics_pipeline_library)
Uses the graphics pipeline library extensions to improve run-time pipeline creation. Instead of creating the whole pipeline at once, this sample pre builds shared pipeline parts like like vertex input state and fragment output state. These are then used to create full pipelines at runtime, reducing build times and possible hick-ups.
*/

class GraphicsPipelineLibraryApp final : public EngineApp
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