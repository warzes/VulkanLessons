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

	bool negativeViewport = true;
	float offsety = 0.0f;
	float offsetx = 0.0f;
	int32_t windingOrder = 1;
	int32_t cullMode = (int32_t)VK_CULL_MODE_BACK_BIT;
	int32_t quadType = 0;

	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkDescriptorSetLayout descriptorSetLayout;
	struct DescriptorSets {
		VkDescriptorSet CW;
		VkDescriptorSet CCW;
	} descriptorSets;

	struct Textures {
		vks::Texture2D CW;
		vks::Texture2D CCW;
	} textures;

	struct Quad {
		VulkanBuffer verticesYUp;
		VulkanBuffer verticesYDown;
		VulkanBuffer indicesCCW;
		VulkanBuffer indicesCW;
		void destroy()
		{
			verticesYUp.destroy();
			verticesYDown.destroy();
			indicesCCW.destroy();
			indicesCW.destroy();
		}
	} quad;

	void setEnabledInstanceExtensions()
	{
		// [POI] VK_KHR_MAINTENANCE1 is required for using negative viewport heights
		// Note: This is core as of Vulkan 1.1. So if you target 1.1 you don't have to explicitly enable this
		enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
	}

	void buildCommandBuffers()
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

		for (int32_t i = 0; i < drawCommandBuffers.size(); ++i) {
			renderPassBeginInfo.framebuffer = frameBuffers[i];

			VK_CHECK_RESULT(vkBeginCommandBuffer(drawCommandBuffers[i], &cmdBufInfo));

			vkCmdBeginRenderPass(drawCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

			// [POI] Viewport setup
			VkViewport viewport{};
			if (negativeViewport) {
				viewport.x = offsetx;
				// [POI] When using a negative viewport height, the origin needs to be adjusted too
				viewport.y = (float)destHeight - offsety;
				viewport.width = (float)destWidth;
				// [POI] Flip the sign of the viewport's height
				viewport.height = -(float)destHeight;
			}
			else {
				viewport.x = offsetx;
				viewport.y = offsety;
				viewport.width = (float)destWidth;
				viewport.height = (float)destHeight;
			}
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(drawCommandBuffers[i], 0, 1, &viewport);

			VkRect2D scissor = vks::initializers::rect2D(destWidth, destHeight, 0, 0);
			vkCmdSetScissor(drawCommandBuffers[i], 0, 1, &scissor);

			VkDeviceSize offsets[1] = { 0 };

			// Render the quad with clock wise and counter clock wise indices, visibility is determined by pipeline settings

			vkCmdBindDescriptorSets(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets.CW, 0, nullptr);
			vkCmdBindIndexBuffer(drawCommandBuffers[i], quad.indicesCW.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdBindVertexBuffers(drawCommandBuffers[i], 0, 1, quadType == 0 ? &quad.verticesYDown.buffer : &quad.verticesYUp.buffer, offsets);
			vkCmdDrawIndexed(drawCommandBuffers[i], 6, 1, 0, 0, 0);

			vkCmdBindDescriptorSets(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets.CCW, 0, nullptr);
			vkCmdBindIndexBuffer(drawCommandBuffers[i], quad.indicesCCW.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(drawCommandBuffers[i], 6, 1, 0, 0, 0);

			DrawUI(drawCommandBuffers[i]);

			vkCmdEndRenderPass(drawCommandBuffers[i]);

			VK_CHECK_RESULT(vkEndCommandBuffer(drawCommandBuffers[i]));
		}
	}

	void loadAssets()
	{
		textures.CW.loadFromFile(getAssetPath() + "textures/texture_orientation_cw_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, m_vulkanDevice, queue);
		textures.CCW.loadFromFile(getAssetPath() + "textures/texture_orientation_ccw_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, m_vulkanDevice, queue);

		// [POI] Create two quads with different Y orientations

		struct Vertex {
			float pos[3];
			float uv[2];
		};

		const float ar = (float)destHeight / (float)destWidth;

		// OpenGL style (y points upwards)
		std::vector<Vertex> verticesYPos = {
			{ -1.0f * ar,  1.0f, 1.0f, 0.0f, 1.0f },
			{ -1.0f * ar, -1.0f, 1.0f, 0.0f, 0.0f },
			{  1.0f * ar, -1.0f, 1.0f, 1.0f, 0.0f },
			{  1.0f * ar,  1.0f, 1.0f, 1.0f, 1.0f },
		};

		// Vulkan style (y points downwards)
		std::vector<Vertex> verticesYNeg = {
			{ -1.0f * ar, -1.0f, 1.0f, 0.0f, 1.0f },
			{ -1.0f * ar,  1.0f, 1.0f, 0.0f, 0.0f },
			{  1.0f * ar,  1.0f, 1.0f, 1.0f, 0.0f },
			{  1.0f * ar, -1.0f, 1.0f, 1.0f, 1.0f },
		};

		const VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		VK_CHECK_RESULT(m_vulkanDevice->createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, memoryPropertyFlags, &quad.verticesYUp, sizeof(Vertex) * 4, verticesYPos.data()));
		VK_CHECK_RESULT(m_vulkanDevice->createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, memoryPropertyFlags, &quad.verticesYDown, sizeof(Vertex) * 4, verticesYNeg.data()));

		// [POI] Create two set of indices, one for counter clock wise, and one for clock wise rendering
		std::vector<uint32_t> indices = { 2,1,0, 0,3,2 };
		VK_CHECK_RESULT(m_vulkanDevice->createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, memoryPropertyFlags, &quad.indicesCCW, indices.size() * sizeof(uint32_t), indices.data()));
		indices = { 0,1,2, 2,3,0 };
		VK_CHECK_RESULT(m_vulkanDevice->createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, memoryPropertyFlags, &quad.indicesCW, indices.size() * sizeof(uint32_t), indices.data()));
	}

	void setupDescriptors()
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0)
		};
		VkDescriptorSetLayoutCreateInfo descriptorLayoutCI = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayoutCI, nullptr, &descriptorSetLayout));
		VkPipelineLayoutCreateInfo pipelineLayoutCI = vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelineLayout));

		VkDescriptorPoolSize poolSize = vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2);
		VkDescriptorPoolCreateInfo descriptorPoolCI = vks::initializers::descriptorPoolCreateInfo(1, &poolSize, 2);
		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &descriptorPool));

		VkDescriptorSetAllocateInfo descriptorSetAI = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);

		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAI, &descriptorSets.CW));
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAI, &descriptorSets.CCW));

		std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			vks::initializers::writeDescriptorSet(descriptorSets.CW, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &textures.CW.descriptor),
			vks::initializers::writeDescriptorSet(descriptorSets.CCW, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &textures.CCW.descriptor)
		};
		vkUpdateDescriptorSets(device, 2, &writeDescriptorSets[0], 0, nullptr);
	}

	void preparePipelines()
	{
		if (pipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(device, pipeline, nullptr);
		}

		const std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
		VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
		VkPipelineColorBlendStateCreateInfo colorBlendStateCI = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
		VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo viewportStateCI = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
		VkPipelineMultisampleStateCreateInfo multisampleStateCI = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
		VkPipelineDynamicStateCreateInfo dynamicStateCI = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables.data(), static_cast<uint32_t>(dynamicStateEnables.size()), 0);

		VkPipelineRasterizationStateCreateInfo rasterizationStateCI{};
		rasterizationStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationStateCI.lineWidth = 1.0f;
		rasterizationStateCI.cullMode = VK_CULL_MODE_NONE + cullMode;
		rasterizationStateCI.frontFace = windingOrder == 0 ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;

		// Vertex bindings and attributes
		std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
			vks::initializers::vertexInputBindingDescription(0, sizeof(float) * 5, VK_VERTEX_INPUT_RATE_VERTEX),
		};
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
			vks::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),				// Position
			vks::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 3),	// uv
		};
		VkPipelineVertexInputStateCreateInfo vertexInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
		vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
		vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
		vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
		vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

		VkGraphicsPipelineCreateInfo pipelineCreateInfoCI = vks::initializers::pipelineCreateInfo(pipelineLayout, renderPass, 0);
		//pipelineCreateInfoCI.pVertexInputState = &emptyInputState;
		pipelineCreateInfoCI.pVertexInputState = &vertexInputState;
		pipelineCreateInfoCI.pInputAssemblyState = &inputAssemblyStateCI;
		pipelineCreateInfoCI.pRasterizationState = &rasterizationStateCI;
		pipelineCreateInfoCI.pColorBlendState = &colorBlendStateCI;
		pipelineCreateInfoCI.pMultisampleState = &multisampleStateCI;
		pipelineCreateInfoCI.pViewportState = &viewportStateCI;
		pipelineCreateInfoCI.pDepthStencilState = &depthStencilStateCI;
		pipelineCreateInfoCI.pDynamicState = &dynamicStateCI;

		const std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
			loadShader(getShadersPath() + "negativeviewportheight/quad.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
			loadShader(getShadersPath() + "negativeviewportheight/quad.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
		};

		pipelineCreateInfoCI.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCreateInfoCI.pStages = shaderStages.data();

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfoCI, nullptr, &pipeline));
	}

	void draw()
	{
		prepareFrame();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &drawCommandBuffers[currentBuffer];
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
		submitFrame();
	}
};