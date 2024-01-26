#pragma once
#include "Game/GameCommon.hpp"
#include "Block.hpp"
#include "Engine/Math/AABB3.hpp"
#include <vector>
#include "World.hpp"
#include "Engine/Math/IntVec3.hpp"
#include <atomic>


constexpr float HUMIDITY_THRESHOLD = 0.4f;
constexpr float TEMP_THRESHOLD = 0.4f;
constexpr float SEA_LEVEL = CHUNK_BLOCK_NUM_OF_Z * 0.5f;
constexpr float MAX_DEPTH_SEA = SEA_LEVEL - 30.f;
constexpr float RIVER_WIDTH = 5; 
constexpr float BEACH_THRESHOLD = 0.6f;
constexpr float CAVE_STEP_LENGTH = 5.f;
constexpr float CAVE_MAX_TURN_DEGREES = 60.f;
constexpr float CAVE_CAPSULE_RADIUS = 4.f;

enum class ChunkState
{
	MISSING,
	ON_DISK,
	CONSTRUCTING,

	ACTIVATING_QUEUED_LOAD,
	ACTIVATING_LOADING,
	ACTIVATING_LOAD_COMPLETE,
	
	ACTIVATING_QUEUED_GENERATE,
	ACTIVATING_GENERRATING,
	ACTIVATING_GENERATE_COMPLETE,

	ACTIVE,

	DEACTIVATING_QUEUED_SAVE,
	DEACTIVATING_SAVING,
	DEACTIVATING_SAVE_COMPLETE,
	DECONSTRUCTING,

	NUM_CHUNK_STATES
};


struct CaveInfo
{
public:
	CaveInfo(IntVec2 const& coords);
	IntVec2 m_startChunkCoords;
	Vec3 m_startGlobalPos;
	std::vector<Vec3> m_caveNodePositions;
};

struct CaveCapsule
{
	Vec3 start;
	Vec3 end;
};

class Chunk
{
public:
	Chunk(World* world, IntVec2 coords);
	~Chunk();
	void Startup();
	void Update(float deltaSeconds);
	void Render() const;
	void EndFrame();
	void Shutdown();

	void InitializedBlocks();
	void RefreshCPUMesh();
	bool ItHasToBeActivated(Vec2 PosXY);
	float GetDistFromPos(Vec2 PosXY);
	bool ItHasToBeDeactivated(Vec2 PosXY);
	void MarkDirty();
	void GenerateChunk();
	void PutBlockTypeOnTop(Vec2 PosXY, uint8_t type);
	int GetIndexForWorldCoords(IntVec3 const& coords) const;

	IntVec3 GetWorldCoordsForIndex(int index) const;
	IntVec3 GetLocalCoordsForIndex(int index) const;
	IntVec3 GetLocalCoordsForWorldCoords(IntVec3 const& coords) const;
	int GetIndexForLocalCoords(IntVec3 const& coords) const;
	void DigBlockTypeOnTop(Vec2 PosXY, uint8_t type);

	size_t GetSizeOfCPUMesh() const;
	Block* GetBlockByIndex(int index) const;
	void MarkNeighborDirtyIfTouchingNonOpaqueBoundary();
	void DescendEachColumnFlaging_SKY();
	void HorizontalSpread();
	void MarkAllEmitsLightBlockDirty();

	IntVec2 GetCoords() const;
	IntVec2 GetNorth() const;
	IntVec2 GetSouth() const;
	IntVec2 GetEast() const;
	IntVec2 GetWest() const;

public:
	Chunk* m_northNeighbor = nullptr;
	Chunk* m_southNeighbor = nullptr;
	Chunk* m_eastNeighbor = nullptr;
	Chunk* m_westNeighbor = nullptr;
	bool m_needsSaving = false;
	Block* m_blocks = nullptr;
	std::atomic<ChunkState> m_chunkState;
	bool m_isVertexDirty = false;
private:
	World* m_theWorld = nullptr;
	IntVec2 m_chunkCoords;
	AABB3 m_bounds;
	std::vector<Vertex_PCU> m_cpuMesh;
	std::vector<IntVec2> m_chunkCroodsWhereCaveStart;
	std::vector<CaveInfo>	 m_nearbyCaves;
	std::vector<CaveCapsule> m_caves;
	VertexBuffer* m_gpuMesh = nullptr;

	
	void SaveData();
	void GenerateTreeAt(IntVec2 currentCoords);
	void GenerateHouseAt();
	bool IsLocalCoordsTreenessLocalMax(IntVec2 currentCoords, std::map<IntVec2, float> const& treenessMap);
	void AddCaves(RandomNumberGenerator& rng, unsigned int seed);
	void DebugAddVertsForCaves(std::vector<Vertex_PCU>& verts) const;
	bool IsCoordsVillageLocalMax(std::map<IntVec2, float> const& houseNoise) const;
	AABB3 GetChunkBoundsForChunkCoords(IntVec2 m_startChunkCoords);
};