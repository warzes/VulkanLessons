#pragma once

/*
Distance field fonts
Uses a texture that stores signed distance field information per character along with a special fragment shader calculating output based on that distance data. This results in crisp high quality font rendering independent of font size and scale.

* This sample compares rendering resolution independent fonts using signed distance fields to traditional bitmap fonts
*
* Font generated using https://github.com/libgdx/libgdx/wiki/Hiero
*/

class DistanceFieldFontsApp final : public EngineApp
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

	// Vertex layout for this example
	struct Vertex {
		float pos[3];
		float uv[2];
	};

	// AngelCode .fnt format structs and classes
	struct bmchar {
		uint32_t x, y;
		uint32_t width;
		uint32_t height;
		int32_t xoffset;
		int32_t yoffset;
		int32_t xadvance;
		uint32_t page;
	};

	// Quick and dirty : We store complete ASCII table
	// Only chars present in the .fnt are filled with data, so not a complete, production ready solution!
	std::array<bmchar, 255> fontChars;

	int32_t nextValuePair(std::stringstream* stream)
	{
		std::string pair;
		*stream >> pair;
		size_t spos = pair.find("=");
		std::string value = pair.substr(spos + 1);
		int32_t val = std::stoi(value);
		return val;
	}

	bool splitScreen = true;

	struct Textures {
		vks::Texture2D fontSDF;
		vks::Texture2D fontBitmap;
	} textures;

	VulkanBuffer vertexBuffer;
	VulkanBuffer indexBuffer;
	uint32_t indexCount{ 0 };

	struct UniformData {
		// Scene matrices
		glm::mat4 projection;
		glm::mat4 modelView;
		// Font display options
		glm::vec4 outlineColor{ 1.0f, 0.0f, 0.0f, 0.0f };
		float outlineWidth{ 0.6f };
		float outline{ true };
	} uniformData;
	VulkanBuffer uniformBuffer;

	struct Pipelines {
		VkPipeline sdf{ VK_NULL_HANDLE };
		VkPipeline bitmap{ VK_NULL_HANDLE };
	} pipelines;

	struct DescriptorSets {
		VkDescriptorSet sdf{ VK_NULL_HANDLE };
		VkDescriptorSet bitmap{ VK_NULL_HANDLE };
	} descriptorSets;

	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };

	// Basic parser for AngelCode bitmap font format files
	// See http://www.angelcode.com/products/bmfont/doc/file_format.html for details
	void parsebmFont()
	{
		std::string fileName = getAssetPath() + "font.fnt";

#if defined(__ANDROID__)
		// Font description file is stored inside the apk
		// So we need to load it using the asset manager
		AAsset* asset = AAssetManager_open(androidApp->activity->assetManager, fileName.c_str(), AASSET_MODE_STREAMING);
		assert(asset);
		size_t size = AAsset_getLength(asset);

		assert(size > 0);

		void* fileData = malloc(size);
		AAsset_read(asset, fileData, size);
		AAsset_close(asset);

		std::stringbuf sbuf((const char*)fileData);
		std::istream istream(&sbuf);
#else
		std::filebuf fileBuffer;
		fileBuffer.open(fileName, std::ios::in);
		std::istream istream(&fileBuffer);
#endif

		assert(istream.good());

		while (!istream.eof())
		{
			std::string line;
			std::stringstream lineStream;
			std::getline(istream, line);
			lineStream << line;

			std::string info;
			lineStream >> info;

			if (info == "char")
			{
				// char id
				uint32_t charid = nextValuePair(&lineStream);
				// Char properties
				fontChars[charid].x = nextValuePair(&lineStream);
				fontChars[charid].y = nextValuePair(&lineStream);
				fontChars[charid].width = nextValuePair(&lineStream);
				fontChars[charid].height = nextValuePair(&lineStream);
				fontChars[charid].xoffset = nextValuePair(&lineStream);
				fontChars[charid].yoffset = nextValuePair(&lineStream);
				fontChars[charid].xadvance = nextValuePair(&lineStream);
				fontChars[charid].page = nextValuePair(&lineStream);
			}
		}

	}

	void loadAssets()
	{
		textures.fontSDF.loadFromFile(getAssetPath() + "textures/font_sdf_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, m_vulkanDevice, queue);
		textures.fontBitmap.loadFromFile(getAssetPath() + "textures/font_bitmap_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, m_vulkanDevice, queue);
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

		for (int32_t i = 0; i < drawCommandBuffers.size(); ++i)
		{
			renderPassBeginInfo.framebuffer = frameBuffers[i];

			VK_CHECK_RESULT(vkBeginCommandBuffer(drawCommandBuffers[i], &cmdBufInfo));

			vkCmdBeginRenderPass(drawCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = vks::initializers::viewport((float)destWidth, (splitScreen) ? (float)destHeight / 2.0f : (float)destHeight, 0.0f, 1.0f);
			vkCmdSetViewport(drawCommandBuffers[i], 0, 1, &viewport);

			VkRect2D scissor = vks::initializers::rect2D(destWidth, destHeight, 0, 0);
			vkCmdSetScissor(drawCommandBuffers[i], 0, 1, &scissor);

			VkDeviceSize offsets[1] = { 0 };

			// Signed distance field font
			vkCmdBindDescriptorSets(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets.sdf, 0, NULL);
			vkCmdBindPipeline(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.sdf);
			vkCmdBindVertexBuffers(drawCommandBuffers[i], 0, 1, &vertexBuffer.buffer, offsets);
			vkCmdBindIndexBuffer(drawCommandBuffers[i], indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(drawCommandBuffers[i], indexCount, 1, 0, 0, 0);

			// Linear filtered bitmap font
			if (splitScreen)
			{
				viewport.y = (float)destHeight / 2.0f;
				vkCmdSetViewport(drawCommandBuffers[i], 0, 1, &viewport);
				vkCmdBindDescriptorSets(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets.bitmap, 0, NULL);
				vkCmdBindPipeline(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.bitmap);
				vkCmdDrawIndexed(drawCommandBuffers[i], indexCount, 1, 0, 0, 0);
			}

			DrawUI(drawCommandBuffers[i]);

			vkCmdEndRenderPass(drawCommandBuffers[i]);

			VK_CHECK_RESULT(vkEndCommandBuffer(drawCommandBuffers[i]));
		}
	}

	// Creates a vertex and index buffer with triangle data containing the chars of the given text
	void generateText(std::string text)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		uint32_t indexOffset = 0;

		float w = static_cast<float>(textures.fontSDF.width);

		float posx = 0.0f;
		float posy = 0.0f;

		for (uint32_t i = 0; i < text.size(); i++)
		{
			bmchar* charInfo = &fontChars[(int)text[i]];

			if (charInfo->width == 0)
				charInfo->width = 36;

			float charw = ((float)(charInfo->width) / 36.0f);
			float dimx = 1.0f * charw;
			float charh = ((float)(charInfo->height) / 36.0f);
			float dimy = 1.0f * charh;

			float us = charInfo->x / w;
			float ue = (charInfo->x + charInfo->width) / w;
			float ts = charInfo->y / w;
			float te = (charInfo->y + charInfo->height) / w;

			float xo = charInfo->xoffset / 36.0f;
			float yo = charInfo->yoffset / 36.0f;

			posy = yo;

			vertices.push_back({ { posx + dimx + xo,  posy + dimy, 0.0f }, { ue, te } });
			vertices.push_back({ { posx + xo,         posy + dimy, 0.0f }, { us, te } });
			vertices.push_back({ { posx + xo,         posy,        0.0f }, { us, ts } });
			vertices.push_back({ { posx + dimx + xo,  posy,        0.0f }, { ue, ts } });

			std::array<uint32_t, 6> letterIndices = { 0,1,2, 2,3,0 };
			for (auto& index : letterIndices)
			{
				indices.push_back(indexOffset + index);
			}
			indexOffset += 4;

			float advance = ((float)(charInfo->xadvance) / 36.0f);
			posx += advance;
		}
		indexCount = static_cast<uint32_t>(indices.size());

		// Center
		for (auto& v : vertices)
		{
			v.pos[0] -= posx / 2.0f;
			v.pos[1] -= 0.5f;
		}

		// Generate host accessible buffers for the text vertices and indices and upload the data
		VK_CHECK_RESULT(m_vulkanDevice->createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexBuffer, vertices.size() * sizeof(Vertex), vertices.data()));
		VK_CHECK_RESULT(m_vulkanDevice->createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &indexBuffer, indices.size() * sizeof(uint32_t), indices.data()));
	}

	void setupDescriptors()
	{
		// Pool
		std::vector<VkDescriptorPoolSize> poolSizes = {
			vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2),
			vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2)
		};
		VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 2);
		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));

		// Layout

		// The pipeline uses one set and three bindings
		// Binding 0 : Shader uniform buffer
		// Binding 1 : Fragment shader image sampler

		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0),
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
		};
		VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));

		// Sets
		VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);

		// Signed distance font descriptor set
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSets.sdf));
		std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			vks::initializers::writeDescriptorSet(descriptorSets.sdf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffer.descriptor),
			vks::initializers::writeDescriptorSet(descriptorSets.sdf, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &textures.fontSDF.descriptor),
		};
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

		// Bitmap font descriptor set
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSets.bitmap));
		writeDescriptorSets = {
			vks::initializers::writeDescriptorSet(descriptorSets.bitmap, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffer.descriptor),
			vks::initializers::writeDescriptorSet(descriptorSets.bitmap, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &textures.fontBitmap.descriptor)
		};
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	}

	void preparePipelines()
	{
		// Layout
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

		// Pipelines
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
		VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_TRUE);
		VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
		VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
		VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

		// Enabled blending (With the background)
		blendAttachmentState.blendEnable = VK_TRUE;
		blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		// Vertex input state
		std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
			vks::initializers::vertexInputBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
		};
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
			vks::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)),
			vks::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)),
		};
		VkPipelineVertexInputStateCreateInfo vertexInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
		vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
		vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
		vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
		vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo(pipelineLayout, renderPass, 0);
		pipelineCreateInfo.pVertexInputState = &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();

		// Signed distance font rendering pipeline
		shaderStages[0] = loadShader(getShadersPath() + "distancefieldfonts/sdf.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = loadShader(getShadersPath() + "distancefieldfonts/sdf.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.sdf));

		// Bitmap font rendering pipeline
		shaderStages[0] = loadShader(getShadersPath() + "distancefieldfonts/bitmap.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = loadShader(getShadersPath() + "distancefieldfonts/bitmap.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.bitmap));
	}

	// Prepare and initialize uniform buffer containing shader uniforms
	void prepareUniformBuffers()
	{
		// Vertex shader uniform buffer block
		VK_CHECK_RESULT(m_vulkanDevice->createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &uniformBuffer, sizeof(UniformData)));
		VK_CHECK_RESULT(uniformBuffer.map());
		updateUniformBuffers();
	}

	void updateUniformBuffers()
	{
		// Adjust camera perspective as we render two viewports
		camera.setPerspective(splitScreen ? 30.0f : 45.0f, (float)destWidth / (float)(destHeight * ((splitScreen) ? 0.5f : 1.0f)), 1.0f, 256.0f);
		uniformData.projection = camera.matrices.perspective;
		uniformData.modelView = camera.matrices.view;
		memcpy(uniformBuffer.mapped, &uniformData, sizeof(UniformData));
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