#pragma once

/*
Implements a simple CPU based particle system. Particle data is stored in host memory, updated on the CPU per-frame and synchronized with the device before it's rendered using pre-multiplied alpha.
This sample renders a particle system that is updated on the host (by the CPU) and rendered by the GPU using a vertex buffer
*/

class ParticleSystemApp final : public EngineApp
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