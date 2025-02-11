#include "stdafx.h"
#include "10_TextureCubemapArray.h"
//-----------------------------------------------------------------------------
bool TextureCubemapArrayApp::OnCreate()
{
	camera.type = Camera::CameraType::lookat;
	camera.setPosition(glm::vec3(0.0f, 0.0f, -4.0f));
	camera.setRotationSpeed(0.25f);
	camera.setPerspective(60.0f, (float)destWidth / (float)destHeight, 0.1f, 256.0f);

	loadAssets();
	prepareUniformBuffers();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();

	return true;
}
//-----------------------------------------------------------------------------
void TextureCubemapArrayApp::OnDestroy()
{
	if (device) {
		vkDestroyImageView(device, cubeMapArray.view, nullptr);
		vkDestroyImage(device, cubeMapArray.image, nullptr);
		vkDestroySampler(device, cubeMapArray.sampler, nullptr);
		vkFreeMemory(device, cubeMapArray.deviceMemory, nullptr);
		vkDestroyPipeline(device, pipelines.skybox, nullptr);
		vkDestroyPipeline(device, pipelines.reflect, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		uniformBuffer.destroy();
	}
}
//-----------------------------------------------------------------------------
void TextureCubemapArrayApp::OnUpdate(float deltaTime)
{
	camera.update(deltaTime);
}
//-----------------------------------------------------------------------------
void TextureCubemapArrayApp::OnFrame()
{
	updateUniformBuffers();
	draw();
}
//-----------------------------------------------------------------------------
void TextureCubemapArrayApp::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	if (overlay->header("Settings")) {
		overlay->sliderInt("Cube map", &uniformData.cubeMapIndex, 0, cubeMapArray.layerCount - 1);
		overlay->sliderFloat("LOD bias", &uniformData.lodBias, 0.0f, (float)cubeMapArray.mipLevels);
		if (overlay->comboBox("Object type", &models.objectIndex, objectNames)) {
			buildCommandBuffers();
		}
		if (overlay->checkBox("Skybox", &displaySkybox)) {
			buildCommandBuffers();
		}
	}
}
//-----------------------------------------------------------------------------
void TextureCubemapArrayApp::OnWindowResize(uint32_t destWidth, uint32_t destHeight)
{
	if ((destWidth > 0.0f) && (destHeight > 0.0f))
		camera.updateAspectRatio((float)destWidth / (float)destHeight);
}
//-----------------------------------------------------------------------------
void TextureCubemapArrayApp::OnKeyPressed(uint32_t key)
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
void TextureCubemapArrayApp::OnKeyUp(uint32_t key)
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
void TextureCubemapArrayApp::OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy)
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
void TextureCubemapArrayApp::getEnabledFeatures()
{
	// This sample requires support for cube map arrays
	if (m_adapter.deviceFeatures.imageCubeArray) {
		enabledFeatures.imageCubeArray = VK_TRUE;
	}
	else {
		vks::tools::exitFatal("Selected GPU does not support cube map arrays!", VK_ERROR_FEATURE_NOT_PRESENT);
	}
	enabledFeatures.imageCubeArray = VK_TRUE;
	if (m_adapter.deviceFeatures.samplerAnisotropy) {
		enabledFeatures.samplerAnisotropy = VK_TRUE;
	}
}
//-----------------------------------------------------------------------------
void TextureCubemapArrayApp::loadCubemapArray(std::string filename, VkFormat format)
{
	ktxResult result;
	ktxTexture* ktxTexture;

#if defined(__ANDROID__)
	// Textures are stored inside the apk on Android (compressed)
	// So they need to be loaded via the asset manager
	AAsset* asset = AAssetManager_open(androidApp->activity->assetManager, filename.c_str(), AASSET_MODE_STREAMING);
	if (!asset) {
		vks::tools::exitFatal("Could not load texture from " + filename + "\n\nMake sure the assets submodule has been checked out and is up-to-date.", -1);
	}
	size_t size = AAsset_getLength(asset);
	assert(size > 0);

	ktx_uint8_t* textureData = new ktx_uint8_t[size];
	AAsset_read(asset, textureData, size);
	AAsset_close(asset);
	result = ktxTexture_CreateFromMemory(textureData, size, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
	delete[] textureData;
#else
	if (!vks::tools::fileExists(filename)) {
		vks::tools::exitFatal("Could not load texture from " + filename + "\n\nMake sure the assets submodule has been checked out and is up-to-date.", -1);
	}
	result = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
#endif
	assert(result == KTX_SUCCESS);

	// Get properties required for using and upload texture data from the ktx texture object
	cubeMapArray.width = ktxTexture->baseWidth;
	cubeMapArray.height = ktxTexture->baseHeight;
	cubeMapArray.mipLevels = ktxTexture->numLevels;
	cubeMapArray.layerCount = ktxTexture->numLayers;
	ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
	ktx_size_t ktxTextureSize = ktxTexture_GetSize(ktxTexture);

	VulkanBuffer sourceData;

	// Create a host-visible source buffer that contains the raw image data
	VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo();
	bufferCreateInfo.size = ktxTextureSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &sourceData.buffer));

	// Get memory requirements for the source buffer (alignment, memory type bits)
	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(device, sourceData.buffer, &memReqs);
	VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
	memAllocInfo.allocationSize = memReqs.size;
	// Get memory type index for a host visible buffer
	memAllocInfo.memoryTypeIndex = m_vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &sourceData.memory));
	VK_CHECK_RESULT(vkBindBufferMemory(device, sourceData.buffer, sourceData.memory, 0));

	// Copy the ktx image data into the source buffer
	uint8_t* data;
	VK_CHECK_RESULT(vkMapMemory(device, sourceData.memory, 0, memReqs.size, 0, (void**)&data));
	memcpy(data, ktxTextureData, ktxTextureSize);
	vkUnmapMemory(device, sourceData.memory);

	// Create optimal tiled target image
	VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = cubeMapArray.mipLevels;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { cubeMapArray.width, cubeMapArray.height, 1 };
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	// Cube faces count as array layers in Vulkan
	imageCreateInfo.arrayLayers = 6 * cubeMapArray.layerCount;
	// This flag is required for cube map images
	imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	VK_CHECK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &cubeMapArray.image));

	// Allocate memory for the cube map array image
	vkGetImageMemoryRequirements(device, cubeMapArray.image, &memReqs);
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = m_vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &cubeMapArray.deviceMemory));
	VK_CHECK_RESULT(vkBindImageMemory(device, cubeMapArray.image, cubeMapArray.deviceMemory, 0));

	/*
		We now copy the parts that make up the cube map array to our image via a command buffer
		Cube map arrays in ktx are stored with a layout like this:
		- Mip Level 0
			- Layer 0 (= Cube map 0)
				- Face +X
				- Face -X
				- Face +Y
				- Face -Y
				- Face +Z
				- Face -Z
			- Layer 1 (= Cube map 1)
				- Face +X
				...
		- Mip Level 1
			- Layer 0 (= Cube map 0)
				- Face +X
				...
			- Layer 1 (= Cube map 1)
				- Face +X
				...
	*/

	VkCommandBuffer copyCmd = m_vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	// Setup buffer copy regions for each face including all of its miplevels
	std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32_t offset = 0;
	for (uint32_t face = 0; face < 6; face++) {
		for (uint32_t layer = 0; layer < ktxTexture->numLayers; layer++) {
			for (uint32_t level = 0; level < ktxTexture->numLevels; level++) {
				ktx_size_t offset;
				KTX_error_code ret = ktxTexture_GetImageOffset(ktxTexture, level, layer, face, &offset);
				assert(ret == KTX_SUCCESS);
				VkBufferImageCopy bufferCopyRegion = {};
				bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				bufferCopyRegion.imageSubresource.mipLevel = level;
				bufferCopyRegion.imageSubresource.baseArrayLayer = layer * 6 + face;
				bufferCopyRegion.imageSubresource.layerCount = 1;
				bufferCopyRegion.imageExtent.width = ktxTexture->baseWidth >> level;
				bufferCopyRegion.imageExtent.height = ktxTexture->baseHeight >> level;
				bufferCopyRegion.imageExtent.depth = 1;
				bufferCopyRegion.bufferOffset = offset;
				bufferCopyRegions.push_back(bufferCopyRegion);
			}
		}
	}

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = cubeMapArray.mipLevels;
	subresourceRange.layerCount = 6 * cubeMapArray.layerCount;

	// Transition target image to accept the writes from our buffer to image copies
	vks::tools::setImageLayout(copyCmd, cubeMapArray.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

	// Copy the cube map array buffer parts from the staging buffer to the optimal tiled image
	vkCmdCopyBufferToImage(
		copyCmd,
		sourceData.buffer,
		cubeMapArray.image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32_t>(bufferCopyRegions.size()),
		bufferCopyRegions.data()
	);

	// Transition image to shader read layout
	cubeMapArray.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	vks::tools::setImageLayout(copyCmd, cubeMapArray.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cubeMapArray.imageLayout, subresourceRange);

	m_vulkanDevice->flushCommandBuffer(copyCmd, queue, true);

	// Create sampler
	VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.compareOp = VK_COMPARE_OP_NEVER;
	sampler.minLod = 0.0f;
	sampler.maxLod = static_cast<float>(cubeMapArray.mipLevels);
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	sampler.maxAnisotropy = 1.0f;
	if (m_vulkanDevice->features.samplerAnisotropy)
	{
		sampler.maxAnisotropy = m_vulkanDevice->properties.limits.maxSamplerAnisotropy;
		sampler.anisotropyEnable = VK_TRUE;
	}
	VK_CHECK_RESULT(vkCreateSampler(device, &sampler, nullptr, &cubeMapArray.sampler));

	// Create the image view for a cube map array
	VkImageViewCreateInfo view = vks::initializers::imageViewCreateInfo();
	view.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
	view.format = format;
	view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	view.subresourceRange.layerCount = 6 * cubeMapArray.layerCount;
	view.subresourceRange.levelCount = cubeMapArray.mipLevels;
	view.image = cubeMapArray.image;
	VK_CHECK_RESULT(vkCreateImageView(device, &view, nullptr, &cubeMapArray.view));

	// Clean up staging resources
	vkFreeMemory(device, sourceData.memory, nullptr);
	vkDestroyBuffer(device, sourceData.buffer, nullptr);
	ktxTexture_Destroy(ktxTexture);
}
//-----------------------------------------------------------------------------
void TextureCubemapArrayApp::buildCommandBuffers()
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
		// Set target frame buffer
		renderPassBeginInfo.framebuffer = frameBuffers[i];

		VK_CHECK_RESULT(vkBeginCommandBuffer(drawCommandBuffers[i], &cmdBufInfo));

		vkCmdBeginRenderPass(drawCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = vks::initializers::viewport((float)destWidth, (float)destHeight, 0.0f, 1.0f);
		vkCmdSetViewport(drawCommandBuffers[i], 0, 1, &viewport);

		VkRect2D scissor = vks::initializers::rect2D(destWidth, destHeight, 0, 0);
		vkCmdSetScissor(drawCommandBuffers[i], 0, 1, &scissor);

		vkCmdBindDescriptorSets(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

		// Skybox
		if (displaySkybox)
		{
			vkCmdBindPipeline(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.skybox);
			models.skybox.draw(drawCommandBuffers[i]);
		}

		// 3D object
		vkCmdBindPipeline(drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.reflect);
		models.objects[models.objectIndex].draw(drawCommandBuffers[i]);

		DrawUI(drawCommandBuffers[i]);

		vkCmdEndRenderPass(drawCommandBuffers[i]);

		VK_CHECK_RESULT(vkEndCommandBuffer(drawCommandBuffers[i]));
	}
}
//-----------------------------------------------------------------------------
void TextureCubemapArrayApp::loadAssets()
{
	uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::FlipY;
	// Skybox
	models.skybox.loadFromFile(getAssetPath() + "models/cube.gltf", m_vulkanDevice, queue, glTFLoadingFlags);
	// Objects
	std::vector<std::string> filenames = { "sphere.gltf", "teapot.gltf", "torusknot.gltf", "venus.gltf" };
	objectNames = { "Sphere", "Teapot", "Torusknot", "Venus" };
	models.objects.resize(filenames.size());
	for (size_t i = 0; i < filenames.size(); i++) {
		models.objects[i].loadFromFile(getAssetPath() + "models/" + filenames[i], m_vulkanDevice, queue, glTFLoadingFlags);
	}
	// Load the cube map array from a ktx texture file
	loadCubemapArray(getAssetPath() + "textures/cubemap_array.ktx", VK_FORMAT_R8G8B8A8_UNORM);
}
//-----------------------------------------------------------------------------
void TextureCubemapArrayApp::setupDescriptors()
{
	// Pool
	std::vector<VkDescriptorPoolSize> poolSizes = {
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
	};
	VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 2);
	VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));

	// Layout
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		// Binding 0 : Vertex shader uniform buffer
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0),
		// Binding 1 : Fragment shader image sampler
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
	};
	VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));

	// Set
	VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

	// Image descriptor for the cube map array texture
	VkDescriptorImageInfo textureDescriptor = vks::initializers::descriptorImageInfo(cubeMapArray.sampler, cubeMapArray.view, cubeMapArray.imageLayout);

	std::vector<VkWriteDescriptorSet> writeDescriptorSets =
	{
		// Binding 0 : Vertex shader uniform buffer
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffer.descriptor),
		// Binding 1 : Fragment shader cubemap sampler
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &textureDescriptor)
	};
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}
//-----------------------------------------------------------------------------
void TextureCubemapArrayApp::preparePipelines()
{
	// Layout
	const VkPipelineLayoutCreateInfo pipelineLayoutCI = vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelineLayout));

	// Pipelines
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo(pipelineLayout, renderPass, 0);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();
	pipelineCI.pVertexInputState = vkglTF::Vertex::getPipelineVertexInputState({ vkglTF::VertexComponent::Position, vkglTF::VertexComponent::Normal });

	// Skybox pipeline (background cube)
	shaderStages[0] = loadShader(getShadersPath() + "texturecubemaparray/skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = loadShader(getShadersPath() + "texturecubemaparray/skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &pipelines.skybox));

	// Cube map reflect pipeline
	shaderStages[0] = loadShader(getShadersPath() + "texturecubemaparray/reflect.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = loadShader(getShadersPath() + "texturecubemaparray/reflect.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	// Enable depth test and write
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthTestEnable = VK_TRUE;
	// Flip cull mode
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &pipelines.reflect));
}
//-----------------------------------------------------------------------------
void TextureCubemapArrayApp::prepareUniformBuffers()
{
	// Object vertex shader uniform buffer
	VK_CHECK_RESULT(m_vulkanDevice->createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &uniformBuffer, sizeof(UniformData)));
	// Map persistent
	VK_CHECK_RESULT(uniformBuffer.map());
}
//-----------------------------------------------------------------------------
void TextureCubemapArrayApp::updateUniformBuffers()
{
	uniformData.projection = camera.matrices.perspective;
	uniformData.modelView = camera.matrices.view;
	uniformData.inverseModelview = glm::inverse(camera.matrices.view);
	memcpy(uniformBuffer.mapped, &uniformData, sizeof(UniformData));
}
//-----------------------------------------------------------------------------
void TextureCubemapArrayApp::draw()
{
	EngineApp::prepareFrame();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &drawCommandBuffers[currentBuffer];
	VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
	EngineApp::submitFrame();
}
//-----------------------------------------------------------------------------