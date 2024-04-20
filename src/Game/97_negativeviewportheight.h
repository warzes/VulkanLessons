#pragma once

/*
Negative viewport height (VK_KHR_Maintenance1 or Vulkan 1.1)
Shows how to render a scene using a negative viewport height, making the Vulkan render setup more similar to other APIs like OpenGL. Also has several options for changing relevant pipeline state, and displaying meshes with OpenGL or Vulkan style coordinates. Details can be found in this tutorial.
https://www.saschawillems.de/blog/2019/03/29/flipping-the-vulkan-viewport/
*/

class NegativeViewportHeightApp final : public EngineApp
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