#pragma once

/*
Shader objects (VK_EXT_shader_object)
Basic sample showing how to use shader objects that can be used to replace pipeline state objects. Instead of baking all state in a PSO, shaders are explicitly loaded and bound as separate objects and state is set using dynamic state extensions. The sample also stores binary shader objets and loads them on consecutive runs.
*/

class ShaderObjectsApp final : public EngineApp
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