#pragma once

/*
* Vulkan Example - Using dynamic state
*
* This sample demonstrates the use of some of the VK_EXT_dynamic_state extensions
* These allow an application to set some pipeline related state dynamically at drawtime
* instead of having to pre-bake the state into a pipeline
* This can help reduce the number of pipelines required
*/

class DynamicStateApp final : public EngineApp
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

	vkglTF::Model scene;

	struct UniformData {
		glm::mat4 projection;
		glm::mat4 modelView;
		glm::vec4 lightPos{ 0.0f, 2.0f, 1.0f, 0.0f };
	} uniformData;
	VulkanBuffer uniformBuffer;

	float clearColor[4] = { 0.0f, 0.0f, 0.2f, 1.0f };

	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	VkPipeline pipeline{ VK_NULL_HANDLE };
	VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };

	// This sample demonstrates different dynamic states, so we check and store what extension is available
	bool hasDynamicState{ false };
	bool hasDynamicState2{ false };
	bool hasDynamicState3{ false };
	bool hasDynamicVertexState{ false };

	VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeaturesEXT{};
	VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extendedDynamicState2FeaturesEXT{};
	VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extendedDynamicState3FeaturesEXT{};

	// Function pointers for dynamic states used in this sample
	// VK_EXT_dynamic_stte
	PFN_vkCmdSetCullModeEXT vkCmdSetCullModeEXT{ nullptr };
	PFN_vkCmdSetFrontFaceEXT vkCmdSetFrontFaceEXT{ nullptr };
	PFN_vkCmdSetDepthTestEnableEXT vkCmdSetDepthTestEnableEXT{ nullptr };
	PFN_vkCmdSetDepthWriteEnableEXT vkCmdSetDepthWriteEnableEXT{ nullptr };
	// VK_EXT_dynamic_state_2
	PFN_vkCmdSetRasterizerDiscardEnable vkCmdSetRasterizerDiscardEnableEXT{ nullptr };
	// VK_EXT_dynamic_state_3
	PFN_vkCmdSetColorBlendEnableEXT vkCmdSetColorBlendEnableEXT{ nullptr };
	PFN_vkCmdSetColorBlendEquationEXT vkCmdSetColorBlendEquationEXT{ nullptr };

	// Dynamic state UI toggles
	struct DynamicState {
		int32_t cullMode = VK_CULL_MODE_BACK_BIT;
		int32_t frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		bool depthTest = true;
		bool depthWrite = true;
	} dynamicState;
	struct DynamicState2 {
		bool rasterizerDiscardEnable = false;
	} dynamicState2;
	struct DynamicState3 {
		bool colorBlendEnable = false;
	} dynamicState3;

	void getEnabledExtensions()
	{
		// Check what dynamic states are supported by the current implementation
		hasDynamicState = m_vulkanDevice->extensionSupported(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
		hasDynamicState2 = m_vulkanDevice->extensionSupported(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
		hasDynamicState3 = m_vulkanDevice->extensionSupported(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
		hasDynamicVertexState = m_vulkanDevice->extensionSupported(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);

		// Enable dynamic state extensions if present. This function is called after physical and before logical device creation, so we can enabled extensions based on a list of supported extensions
		if (m_vulkanDevice->extensionSupported(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME)) {
			enabledDeviceExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
			extendedDynamicStateFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
			extendedDynamicStateFeaturesEXT.extendedDynamicState = VK_TRUE;
			deviceCreatepNextChain = &extendedDynamicStateFeaturesEXT;
		}
		if (m_vulkanDevice->extensionSupported(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME)) {
			enabledDeviceExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
			extendedDynamicState2FeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;
			extendedDynamicState2FeaturesEXT.extendedDynamicState2 = VK_TRUE;
			if (hasDynamicState) {
				extendedDynamicStateFeaturesEXT.pNext = &extendedDynamicState2FeaturesEXT;
			}
			else {
				deviceCreatepNextChain = &extendedDynamicState2FeaturesEXT;
			}
		}
		if (m_vulkanDevice->extensionSupported(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME)) {
			enabledDeviceExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
			extendedDynamicState3FeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT;
			extendedDynamicState3FeaturesEXT.extendedDynamicState3ColorBlendEnable = VK_TRUE;
			extendedDynamicState3FeaturesEXT.extendedDynamicState3ColorBlendEquation = VK_TRUE;
			if (hasDynamicState2) {
				extendedDynamicState2FeaturesEXT.pNext = &extendedDynamicState3FeaturesEXT;
			}
			else {
				deviceCreatepNextChain = &extendedDynamicState3FeaturesEXT;
			}

		}
		if (m_vulkanDevice->extensionSupported(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME)) {
			enabledDeviceExtensions.push_back(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
		}
	}

	void buildCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

		VkClearValue clearValues[2];
		clearValues[0].color = { { clearColor[0], clearColor[1], clearColor[2], clearColor[3] } };
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
			// Set target frame buffer
			renderPassBeginInfo.framebuffer = frameBuffers[i];

			VK_CHECK_RESULT(vkBeginCommandBuffer(drawCommandBuffers[i], &cmdBufInfo));

			vkCmdBeginRenderPass(drawCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = vks::initializers::viewport((float)destWidth, (float)destHeight, 0.0f, 1.0f);
			vkCmdSetViewport(drawCommandBuffers[i], 0, 1, &viewport);

			VkRect2D scissor = vks::initializers::rect2D(destWidth, destHeight, 0, 0);
			vkCmdSetScissor(drawCommandBuffers[i], 0, 1, &scissor);

			// Apply dynamic states

			if (vkCmdSetCullModeEXT) {
				vkCmdSetCullModeEXT(drawCommandBuffers[i], VkCullModeFlagBits(dynamicState.cullMode));
			}
			if (vkCmdSetFrontFaceEXT) {
				vkCmdSetFrontFaceEXT(drawCommandBuffers[i], VkFrontFace(dynamicState.frontFace));
			}
			if (vkCmdSetDepthTestEnableEXT) {
				vkCmdSetDepthTestEnableEXT(drawCommandBuffers[i], VkFrontFace(dynamicState.depthTest));
			}
			if (vkCmdSetDepthWriteEnableEXT) {
				vkCmdSetDepthWriteEnableEXT(drawCommandBuffers[i], VkFrontFace(dynamicState.depthWrite));
			}

			if (vkCmdSetRasterizerDiscardEnableEXT) {
				vkCmdSetRasterizerDiscardEnableEXT(drawCommandBuffers[i], VkBool32(dynamicState2.rasterizerDiscardEnable));
			}

			if (vkCmdSetColorBlendEnableEXT) {
				const std::vector<VkBool32> blendEnables = { dynamicState3.colorBlendEnable };
				vkCmdSetColorBlendEnableEXT(drawCommandBuffers[i], 0, 1, blendEnables.data());

				VkColorBlendEquationEXT colorBlendEquation{};

				if (dynamicState3.colorBlendEnable) {
					colorBlendEquation.colorBlendOp = VK_BLEND_OP_ADD;
					colorBlendEquation.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
					colorBlendEquation.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
					colorBlendEquation.alphaBlendOp = VK_BLEND_OP_ADD;
					colorBlendEquation.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					colorBlendEquation.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				}

				vkCmdSetColorBlendEquationEXT(drawCommandBuffers[i], 0, 1, &colorBlendEquation);
			}

			vkCmdBindDescriptorSets(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
			scene.bindBuffers(drawCommandBuffers[i]);

			vkCmdBindPipeline(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			scene.draw(drawCommandBuffers[i]);

			DrawUI(drawCommandBuffers[i]);

			vkCmdEndRenderPass(drawCommandBuffers[i]);

			VK_CHECK_RESULT(vkEndCommandBuffer(drawCommandBuffers[i]));
		}
	}

	void loadAssets()
	{
		const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
		scene.loadFromFile(getAssetPath() + "models/treasure_smooth.gltf", m_vulkanDevice, queue, glTFLoadingFlags);
	}

	void setupDescriptors()
	{
		// Pool
		std::vector<VkDescriptorPoolSize> poolSizes = {
			vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
		};
		VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 2);
		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));

		// Layout
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			// Binding 0 : Vertex shader uniform buffer
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0)
		};
		VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));

		// Set
		VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));
		std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			// Binding 0 : Vertex shader uniform buffer
			vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffer.descriptor)
		};
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	}

	void preparePipelines()
	{
		// Layout
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

		// Pipeline
		// Instead of having to create a pipeline for each state combination, we only create one pipeline and toggle the new dynamic states during command buffer creation
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
		VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
		VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
		VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
		VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

		// All dynamic states we want to use need to be enabled at pipeline creation
		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH, };
		if (hasDynamicState) {
			dynamicStateEnables.push_back(VK_DYNAMIC_STATE_CULL_MODE_EXT);
			dynamicStateEnables.push_back(VK_DYNAMIC_STATE_FRONT_FACE_EXT);
			dynamicStateEnables.push_back(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT);
			dynamicStateEnables.push_back(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT);
		}
		if (hasDynamicState2) {
			dynamicStateEnables.push_back(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT);
		}
		if (hasDynamicState3) {
			dynamicStateEnables.push_back(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
			dynamicStateEnables.push_back(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT);
		}

		VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

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

		// Create the graphics pipeline state objects

		shaderStages[0] = loadShader(getShadersPath() + "pipelines/phong.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = loadShader(getShadersPath() + "pipelines/phong.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &pipeline));
	}

	// Prepare and initialize uniform buffer containing shader uniforms
	void prepareUniformBuffers()
	{
		// Create the vertex shader uniform buffer block
		VK_CHECK_RESULT(m_vulkanDevice->createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &uniformBuffer, sizeof(UniformData)));
		VK_CHECK_RESULT(uniformBuffer.map());
	}

	void updateUniformBuffers()
	{
		uniformData.projection = camera.matrices.perspective;
		uniformData.modelView = camera.matrices.view;
		memcpy(uniformBuffer.mapped, &uniformData, sizeof(uniformData));
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