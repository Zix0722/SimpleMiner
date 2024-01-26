#pragma once
#include <vector>
#include "Engine/Renderer/SpriteSheet.hpp"
#include <map>
#include <deque>
#include "BlockIterator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <string>

class Game;
class Chunk;

constexpr float REAL_TIME_RATIO = 200.f;
constexpr float DAYS_PER_SECOND = 1.f / (60.f * 60.f * 24.f);
constexpr float TIME_SCALE = 50.f;

struct GameRaycastResult3D : public RaycastResult3D
{
public:
	GameRaycastResult3D();
	explicit GameRaycastResult3D(Vec3 const& rayStart, Vec3 const& forwardNormal, float maxLength, bool didImpact, Vec3 const& impactPosition, float impactDist, Vec3 const& impactNormal);
	~GameRaycastResult3D();
public:
	BlockIterator m_impactBlock;
};

struct LockedRaycastData
{
	Vec3 pos;
	Vec3 dir;
	float maxDist;
};

class World
{
public:
	World(Game* theGame);
	~World();
	void Startup();
	void Update(float deltaSeconds);
	void Render() const;
	void EndFrame();
	void Shutdown();

	Chunk* GetChunkAtCoords(IntVec2 const& chunkCoords);
	void DeleteAllChunks();
	void MarkLightingDirtyIfNotOpaque(Block* block, BlockIterator const& iter);
	void MarkLightingDirty(Chunk* chunk, int index);
	IntVec2 GetChunkCoordinatesForPosition(Vec3 const& pos) const;
	Chunk* GetChunkForCoordinate(IntVec2 const& coordinate) const;
	GameRaycastResult3D RaycastVsBlocks(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist);
	void RenderCurrentFaceOfBlock() const;
	void DigRaycastResult();
	void PutRaycastResult(uint8_t type);
	unsigned int GetWorldSeed() const;
	bool CheckIfWorldFolderExist();
	void ForceCreateWorldFolder();
	void RetrieveCompletedJobs();
	void HookUpNeighbors(Chunk* chunk);
public:
	Game* m_theGame = nullptr;
	std::map<IntVec2, Chunk*> m_activeChunks;
	std::vector<Chunk*> m_generatingChunks;
	SpriteSheet* m_worldSpriteSheet = nullptr;
	Texture* m_textureSheet = nullptr;
	float m_chunkActivationRange = 0.f;
	float m_chunkDeactivationRange = 0.f;
	int m_maxChunks = 0;
	std::deque<BlockIterator> m_lightQueue;
	float m_worldTime = 0.5f;
	float m_lightingStrength;
	float m_glowStrength;
	GameRaycastResult3D m_currentPlayerRaycastResult;
	bool m_isRaycastLocked = false;
	LockedRaycastData m_currentLockedData;
	bool m_isAddingDebugRender = false;
	uint8_t m_currentType = 1;
	std::string worldFolder;
private:
	bool ActivateNearestIfAny();
	bool DeactivateFurthestIfAny();
	float GetDistFromIntVec2(Vec2 const& currentPos, IntVec2 const& targetCoords);

	void UnHookNeighbors(Chunk*& deactivateChunk);
	void ProcessDirtyLighting();             
	void ProcessNextDirtyLightBlock();
	void UndirtyAllBlocksInChunk(Chunk* chunk);
	void UpdateCurrentRaycastResult();
	void DebugDrawCavesForAllChunks() const;
};