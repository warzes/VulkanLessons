#include "stdafx.h"
#include "GameApp.h"
//-----------------------------------------------------------------------------
bool GameApp::OnCreate()
{
	camera.type = game::Camera::lookat;
	camera.SetPerspective(60.0f, GetFrameAspect(), 0.1f, 512.0f);
	//camera.SetRotation(glm::vec3(0.0f));
	//camera.SetPosition(glm::vec3(0.0f, 0.0f, -2.0f));
	camera.SetPosition(glm::vec3(0.0f, 0.0f, -10.5f));
	camera.SetRotation(glm::vec3(-25.0f, 15.0f, 0.0f));
	camera.SetRotationSpeed(0.5f);

	scene.loadFromFile(
		"GameData/models/treasure_smooth.gltf", 
		m_vulkanDevice, 
		queue, 
		vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY);

	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void GameApp::OnDestroy()
{
	if (device)
	{
		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		uniformBuffer.destroy();
	}
}
//-----------------------------------------------------------------------------
void GameApp::OnUpdate(float deltaTime)
{
	camera.Update(deltaTime);
}
//-----------------------------------------------------------------------------
void GameApp::OnFrame()
{
	updateUniformBuffers();

	prepareFrame();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &drawCommandBuffers[currentBuffer];
	VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
	submitFrame();
}
//-----------------------------------------------------------------------------
void GameApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
}
//-----------------------------------------------------------------------------
void GameApp::OnWindowResize(uint32_t /*destWidth*/, uint32_t /*destHeight*/)
{
	camera.UpdateAspectRatio(GetFrameAspect());
}
//-----------------------------------------------------------------------------
void GameApp::OnKeyPressed(uint32_t key)
{
	camera.OnKeyPressed(key);
}
//-----------------------------------------------------------------------------
void GameApp::OnKeyUp(uint32_t key)
{
	camera.OnKeyUp(key);	
}
//-----------------------------------------------------------------------------
void GameApp::OnMouseMoved(int32_t /*x*/, int32_t /*y*/, int32_t dx, int32_t dy)
{
	camera.OnMouse(mouseState, dx, dy);
}
//-----------------------------------------------------------------------------
void GameApp::prepareUniformBuffers()
{
	// Create the vertex shader uniform buffer block
	VK_CHECK_RESULT(m_vulkanDevice->createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &uniformBuffer, sizeof(UniformData)));
	// Map persistent
	VK_CHECK_RESULT(uniformBuffer.map());
}
//-----------------------------------------------------------------------------
void GameApp::setupDescriptors()
{
	// Pool
	{
		VkDescriptorPoolSize descriptorPoolSize{};
		descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorPoolSize.descriptorCount = 1;
		const std::vector<VkDescriptorPoolSize> poolSizes = { descriptorPoolSize };

		VkDescriptorPoolCreateInfo descriptorPoolInfo{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = 2;
		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
	}

	// Layout
	{
		// Binding 0 : Vertex shader uniform buffer

		VkDescriptorSetLayoutBinding setLayoutBinding{};
		setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		setLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		setLayoutBinding.binding = 0;
		setLayoutBinding.descriptorCount = 1;
		const std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = { setLayoutBinding };

		VkDescriptorSetLayoutCreateInfo descriptorLayout{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		descriptorLayout.pBindings = setLayoutBindings.data();
		descriptorLayout.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));
	}
	
	// Set
	{
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		descriptorSetAllocateInfo.descriptorPool = descriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
		descriptorSetAllocateInfo.descriptorSetCount = 1;
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));
	}

	VkWriteDescriptorSet writeDescriptorSet{ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	writeDescriptorSet.dstSet = descriptorSet;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.pBufferInfo = &uniformBuffer.descriptor;
	writeDescriptorSet.descriptorCount = 1;

	// Binding 0 : Vertex shader uniform buffer
	const std::vector<VkWriteDescriptorSet> writeDescriptorSets = { writeDescriptorSet };
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}
//-----------------------------------------------------------------------------
void GameApp::preparePipelines()
{
	// Layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

	// Pipelines

	// Most state is shared between all pipelines
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH, };
	VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo(pipelineLayout, renderPass);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();
	pipelineCI.pVertexInputState = vkglTF::Vertex::getPipelineVertexInputState({ vkglTF::VertexComponent::Position, vkglTF::VertexComponent::Normal, vkglTF::VertexComponent::Color });

	shaderStages[0] = loadShader("GameData/Shaders/phong.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = loadShader("GameData/Shaders/phong.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &pipeline));
}
//-----------------------------------------------------------------------------
void GameApp::buildCommandBuffers()
{
	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	VkClearValue clearValues[2];
	clearValues[0].color = defaultClearColor;
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = destWidth;
	renderPassBeginInfo.renderArea.extent.height = destHeight;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	for (int32_t i = 0; i < drawCommandBuffers.size(); ++i)
	{
		renderPassBeginInfo.framebuffer = frameBuffers[i];

		VK_CHECK_RESULT(vkBeginCommandBuffer(drawCommandBuffers[i], &cmdBufInfo));

		vkCmdBeginRenderPass(drawCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = vks::initializers::viewport((float)destWidth, (float)destHeight, 0.0f, 1.0f);
		vkCmdSetViewport(drawCommandBuffers[i], 0, 1, &viewport);

		VkRect2D scissor = vks::initializers::rect2D(destWidth, destHeight, 0, 0);
		vkCmdSetScissor(drawCommandBuffers[i], 0, 1, &scissor);

		vkCmdBindDescriptorSets(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
		scene.bindBuffers(drawCommandBuffers[i]);

		// Left : Render the scene using the solid colored pipeline with phong shading
		vkCmdSetViewport(drawCommandBuffers[i], 0, 1, &viewport);
		vkCmdBindPipeline(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdSetLineWidth(drawCommandBuffers[i], 1.0f);
		scene.draw(drawCommandBuffers[i]);

		DrawUI(drawCommandBuffers[i]);

		vkCmdEndRenderPass(drawCommandBuffers[i]);

		VK_CHECK_RESULT(vkEndCommandBuffer(drawCommandBuffers[i]));
	}
}
//-----------------------------------------------------------------------------
void GameApp::updateUniformBuffers()
{
	uniformData.projection = camera.matrices.perspective;
	uniformData.modelView = camera.matrices.view;
	memcpy(uniformBuffer.mapped, &uniformData, sizeof(UniformData));
}
//-----------------------------------------------------------------------------