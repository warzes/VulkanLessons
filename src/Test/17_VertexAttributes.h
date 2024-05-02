#pragma once

// Passing vertex attributes using interleaved and separate buffers

class VertexAttributesApp final : public EngineApp
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

	struct PushConstBlock {
		glm::mat4 nodeMatrix;
		uint32_t alphaMask;
		float alphaMaskCutoff;
	};

	struct Material {
		glm::vec4 baseColorFactor = glm::vec4(1.0f);
		uint32_t baseColorTextureIndex;
		uint32_t normalTextureIndex;
		std::string alphaMode = "OPAQUE";
		float alphaCutOff;
		VkDescriptorSet descriptorSet;
	};

	struct Image {
		vks::Texture2D texture;
	};

	struct Texture {
		int32_t imageIndex;
	};

	// Layout for the interleaved vertex attributes
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec4 tangent;
	};

	struct Primitive {
		uint32_t firstIndex;
		uint32_t indexCount;
		int32_t materialIndex;
	};
	struct Mesh {
		std::vector<Primitive> primitives;
	};
	struct Node;
	struct Node {
		Node* parent;
		std::vector<Node> children;
		Mesh mesh;
		glm::mat4 matrix;
	};

	std::vector<Node> nodes;

	enum VertexAttributeSettings { interleaved, separate };
	VertexAttributeSettings vertexAttributeSettings = separate;

	// Used to store indices and vertices from glTF to be uploaded to the GPU
	std::vector<uint32_t> indexBuffer;
	std::vector<Vertex> vertexBuffer;
	struct VertexAttributes {
		std::vector<glm::vec2> uv;
		std::vector<glm::vec3> pos, normal;
		std::vector<glm::vec4> tangent;
	} vertexAttributeBuffers;

	// Buffers for the separate vertex attributes
	struct SeparateVertexBuffers {
		VulkanBuffer pos, normal, uv, tangent;
	} separateVertexBuffers;

	// Single vertex buffer for all primitives
	VulkanBuffer interleavedVertexBuffer;

	// Index buffer for all primitives of the scene
	VulkanBuffer indices;

	struct ShaderData {
		VulkanBuffer buffer;
		struct Values {
			glm::mat4 projection;
			glm::mat4 view;
			glm::vec4 lightPos = glm::vec4(0.0f, 2.5f, 0.0f, 1.0f);
			glm::vec4 viewPos;
		} values;
	} shaderData;

	struct Pipelines {
		VkPipeline vertexAttributesInterleaved;
		VkPipeline vertexAttributesSeparate;
	} pipelines;
	VkPipelineLayout pipelineLayout;

	struct DescriptorSetLayouts {
		VkDescriptorSetLayout matrices;
		VkDescriptorSetLayout textures;
	} descriptorSetLayouts;
	VkDescriptorSet descriptorSet;

	struct Scene {
		std::vector<Image> images;
		std::vector<Texture> textures;
		std::vector<Material> materials;
	} scene;

	void getEnabledFeatures() final;
	void buildCommandBuffers() final;
	void uploadVertexData();
	void loadglTFFile(std::string filename);
	void loadAssets();
	void setupDescriptors();
	void preparePipelines();
	void prepareUniformBuffers();
	void updateUniformBuffers();
	void loadSceneNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent);
	void drawSceneNode(VkCommandBuffer commandBuffer, Node node);
};