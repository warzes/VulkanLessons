#pragma once

#include "IPTHeightMapSettings.h"
#include "IPTTerrainChunk.h"

class InfiniteTerrain {
public:
	glm::vec2 viewerPosition;
	int chunkSize;
	int chunksVisibleInViewDistance;

	std::vector<TerrainChunk*> terrainChunks{};
	std::vector<TerrainChunk*> terrainChunkgsUpdateList{};

	InfiniteTerrain();
	void updateViewDistance(float viewDistance);
	bool chunkPresent(glm::ivec2 coords);
	TerrainChunk* getChunk(glm::ivec2 coords);
	TerrainChunk* getChunkFromWorldPos(glm::vec3 coords);
	bool getHeight(const glm::vec3 worldPos, float& height);
	int getVisibleChunkCount();
	int getVisibleTreeCount();
	bool updateVisibleChunks(vks::Frustum& frustum);
	void updateChunks();
	void clear();
	void update(float deltaTime);
};