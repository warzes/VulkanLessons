#include "stdafx.h"
#include "15_ParticleSystem.h"
//-----------------------------------------------------------------------------
bool ParticleSystemApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPosition(glm::vec3(0.0f, 0.0f, -75.0f));
	camera.setRotation(glm::vec3(-15.0f, 45.0f, 0.0f));
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 1.0f, 256.0f);

	timerSpeed *= 8.0f;
	rndEngine.seed(benchmark.active ? 0 : (unsigned)time(nullptr));

	loadAssets();
	prepareParticles();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void ParticleSystemApp::OnDestroy()
{
	if (device) {
		textures.particles.smoke.destroy();
		textures.particles.fire.destroy();
		textures.floor.colorMap.destroy();
		textures.floor.normalMap.destroy();

		vkDestroyPipeline(device, pipelines.particles, nullptr);
		vkDestroyPipeline(device, pipelines.environment, nullptr);

		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		vkUnmapMemory(device, particles.memory);
		vkDestroyBuffer(device, particles.buffer, nullptr);
		vkFreeMemory(device, particles.memory, nullptr);

		uniformBuffers.environment.destroy();
		uniformBuffers.particles.destroy();

		vkDestroySampler(device, textures.particles.sampler, nullptr);
	}
}
//-----------------------------------------------------------------------------
void ParticleSystemApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void ParticleSystemApp::OnFrame()
{
	updateUniformBuffers();
	if (!paused) {
		updateParticles();
	}
	draw();
}
//-----------------------------------------------------------------------------
void ParticleSystemApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{

}
//-----------------------------------------------------------------------------
void ParticleSystemApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void ParticleSystemApp::OnKeyPressed(uint32_t key)
{
	if (key == KEY_F2)
	{
		if (camera.type == Camera::CameraType::lookat)
		{
			camera.type = Camera::CameraType::firstperson;
		}
		else
		{
			camera.type = Camera::CameraType::lookat;
		}
	}

	if (camera.type == Camera::firstperson)
	{
		switch (key)
		{
		case KEY_W:
			camera.keys.up = true;
			break;
		case KEY_S:
			camera.keys.down = true;
			break;
		case KEY_A:
			camera.keys.left = true;
			break;
		case KEY_D:
			camera.keys.right = true;
			break;
		}
	}
}
//-----------------------------------------------------------------------------
void ParticleSystemApp::OnKeyUp(uint32_t key)
{
	if (camera.type == Camera::firstperson)
	{
		switch (key)
		{
		case KEY_W:
			camera.keys.up = false;
			break;
		case KEY_S:
			camera.keys.down = false;
			break;
		case KEY_A:
			camera.keys.left = false;
			break;
		case KEY_D:
			camera.keys.right = false;
			break;
		}
	}
}
//-----------------------------------------------------------------------------
void ParticleSystemApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
{
	if (mouseState.buttons.left)
	{
		camera.rotate(glm::vec3(dy * camera.rotationSpeed, -dx * camera.rotationSpeed, 0.0f));
	}
	if (mouseState.buttons.right)
	{
		camera.translate(glm::vec3(-0.0f, 0.0f, dy * .005f));
	}
	if (mouseState.buttons.middle)
	{
		camera.translate(glm::vec3(-dx * 0.005f, -dy * 0.005f, 0.0f));
	}
}
//-----------------------------------------------------------------------------
