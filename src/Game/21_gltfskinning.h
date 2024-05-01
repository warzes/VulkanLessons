#pragma once

// Contains everything required to render a glTF model in Vulkan
// This class is heavily simplified (compared to glTF's feature set) but retains the basic glTF structure
class VulkanglTFModel2
{
public:
	VulkanDevice* vulkanDevice;
	VkQueue            copyQueue;

	/*
		Base glTF structures, see gltfscene sample for details
	*/

	struct Vertices
	{
		VkBuffer       buffer;
		VkDeviceMemory memory;
	} vertices;

	struct Indices
	{
		int            count;
		VkBuffer       buffer;
		VkDeviceMemory memory;
	} indices;

	struct Node;

	struct Material
	{
		glm::vec4 baseColorFactor = glm::vec4(1.0f);
		uint32_t  baseColorTextureIndex;
	};

	struct Image
	{
		vks::Texture2D  texture;
		VkDescriptorSet descriptorSet;
	};

	struct Texture
	{
		int32_t imageIndex;
	};

	struct Primitive
	{
		uint32_t firstIndex;
		uint32_t indexCount;
		int32_t  materialIndex;
	};

	struct Mesh
	{
		std::vector<Primitive> primitives;
	};

	struct Node
	{
		Node* parent;
		uint32_t            index;
		std::vector<Node*> children;
		Mesh                mesh;
		glm::vec3           translation{};
		glm::vec3           scale{ 1.0f };
		glm::quat           rotation{};
		int32_t             skin = -1;
		glm::mat4           matrix;
		glm::mat4           getLocalMatrix();
	};

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec3 color;
		glm::vec4 jointIndices;
		glm::vec4 jointWeights;
	};

	/*
		Skin structure
	*/

	struct Skin
	{
		std::string            name;
		Node* skeletonRoot = nullptr;
		std::vector<glm::mat4> inverseBindMatrices;
		std::vector<Node*>    joints;
		vks::Buffer            ssbo;
		VkDescriptorSet        descriptorSet;
	};

	/*
		Animation related structures
	*/

	struct AnimationSampler
	{
		std::string            interpolation;
		std::vector<float>     inputs;
		std::vector<glm::vec4> outputsVec4;
	};

	struct AnimationChannel
	{
		std::string path;
		Node* node;
		uint32_t    samplerIndex;
	};

	struct Animation
	{
		std::string                   name;
		std::vector<AnimationSampler> samplers;
		std::vector<AnimationChannel> channels;
		float                         start = std::numeric_limits<float>::max();
		float                         end = std::numeric_limits<float>::min();
		float                         currentTime = 0.0f;
	};

	std::vector<Image>     images;
	std::vector<Texture>   textures;
	std::vector<Material>  materials;
	std::vector<Node*>    nodes;
	std::vector<Skin>      skins;
	std::vector<Animation> animations;

	uint32_t activeAnimation = 0;

	~VulkanglTFModel2();
	void      loadImages(tinygltf::Model& input);
	void      loadTextures(tinygltf::Model& input);
	void      loadMaterials(tinygltf::Model& input);
	Node* findNode(Node* parent, uint32_t index);
	Node* nodeFromIndex(uint32_t index);
	void      loadSkins(tinygltf::Model& input);
	void      loadAnimations(tinygltf::Model& input);
	void      loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, VulkanglTFModel2::Node* parent, uint32_t nodeIndex, std::vector<uint32_t>& indexBuffer, std::vector<VulkanglTFModel2::Vertex>& vertexBuffer);
	glm::mat4 getNodeMatrix(VulkanglTFModel2::Node* node);
	void      updateJoints(VulkanglTFModel2::Node* node);
	void      updateAnimation(float deltaTime);
	void      drawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VulkanglTFModel2::Node node);
	void      draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
};

class GLTFSkinningApp final : public EngineApp
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

	bool wireframe = false;

	struct ShaderData
	{
		vks::Buffer buffer;
		struct Values
		{
			glm::mat4 projection;
			glm::mat4 model;
			glm::vec4 lightPos = glm::vec4(5.0f, 5.0f, 5.0f, 1.0f);
		} values;
	} shaderData;

	VkPipelineLayout pipelineLayout;
	struct Pipelines
	{
		VkPipeline solid{ VK_NULL_HANDLE };
		VkPipeline wireframe{ VK_NULL_HANDLE };
	} pipelines;

	struct DescriptorSetLayouts
	{
		VkDescriptorSetLayout matrices{ VK_NULL_HANDLE };
		VkDescriptorSetLayout textures{ VK_NULL_HANDLE };
		VkDescriptorSetLayout jointMatrices{ VK_NULL_HANDLE };
	} descriptorSetLayouts;
	VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };

	VulkanglTFModel2 glTFModel;

	void         loadglTFFile(std::string filename);
	virtual void getEnabledFeatures();
	void         buildCommandBuffers();
	void         loadAssets();
	void         setupDescriptors();
	void         preparePipelines();
	void         prepareUniformBuffers();
	void         updateUniformBuffers();
};