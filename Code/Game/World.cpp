#include "World.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/Chunk.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "BlockDef.hpp"
#include "ThirdParty/Squirrel/SmoothNoise.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <cmath>
#include "GameJobs.hpp"

GameRaycastResult3D::GameRaycastResult3D()
{

}

GameRaycastResult3D::GameRaycastResult3D(Vec3 const& rayStart, Vec3 const& forwardNormal, float maxLength, bool didImpact, Vec3 const& impactPosition, float impactDist, Vec3 const& impactNormal)
{
	m_rayStartPosition = rayStart;
	m_didImpact = didImpact;
    m_impactDistance = impactDist;
	m_impactPosition = impactPosition;
	m_impactNormal = impactNormal;
	m_rayDirection = forwardNormal;
    m_rayLength = maxLength;
}

GameRaycastResult3D::~GameRaycastResult3D()
{

}

World::World(Game* theGame)
	:m_theGame(theGame)
{
	m_textureSheet = g_theRenderer->CreateOrGetTextureFromFile("Data/Image/BasicSprites_64x64.png");
	m_worldSpriteSheet = new SpriteSheet(*m_textureSheet, IntVec2(64, 64));
	m_chunkActivationRange = theGame->m_app->m_setting.chunkActivationDistance;
 	m_chunkDeactivationRange = m_chunkActivationRange + CHUNK_BLOCK_NUM_OF_X + CHUNK_BLOCK_NUM_OF_Y;
	int maxChunksRadiusX = 1 + int(m_chunkActivationRange) / CHUNK_BLOCK_NUM_OF_X;
	int maxChunksRadiusY = 1 + int(m_chunkActivationRange) / CHUNK_BLOCK_NUM_OF_Y;
	m_maxChunks = (2 * maxChunksRadiusX) * (2 * maxChunksRadiusY);


}

World::~World()
{
	for (auto const& pair : m_activeChunks)
	{
		if (pair.second != nullptr)
		{
			if ((pair.second)->m_needsSaving)
			{
				(pair.second)->~Chunk();
			}
		}
	}
	m_activeChunks.clear();
}

void World::Startup()
{
	if (CheckIfWorldFolderExist())
	{

	}
	else
	{
		ForceCreateWorldFolder();
	}
}

void World::Update(float deltaSeconds)
{
	ProcessDirtyLighting();
	float lightingPerlin = Compute1dPerlinNoise(m_worldTime, 0.008f, 9);
	m_lightingStrength = RangeMapClamped(lightingPerlin, 0.6f, 0.9f, 0.f, 1.f);
	float glowPerlin = Compute1dPerlinNoise(m_worldTime * 50.f, 0.01f, 5);
	m_glowStrength = RangeMapClamped(glowPerlin, -1.f, 1.f, 0.8f, 1.f);

	if (g_theInput->IsKeyDown('Y'))
	{
		m_worldTime += (deltaSeconds * TIME_SCALE * REAL_TIME_RATIO) * DAYS_PER_SECOND;
	}
	else
	{
		m_worldTime += (deltaSeconds  * REAL_TIME_RATIO) * DAYS_PER_SECOND;
	}

	bool didActiveation = false;
	if (m_activeChunks.size() < m_maxChunks)
	{
		didActiveation = ActivateNearestIfAny();
	}
	if (!didActiveation)
	{
		DeactivateFurthestIfAny();
	}

	RetrieveCompletedJobs();

	for (auto const& pair : m_activeChunks)
	{
		if (pair.second != nullptr)
		{
			(pair.second)->Update(deltaSeconds);
		}
	}

	UpdateCurrentRaycastResult();
}

void World::Render() const
{
	for (auto const& pair : m_activeChunks)
	{
		if (pair.second != nullptr)
		{
			(pair.second)->Render();
		}
	}
	RenderCurrentFaceOfBlock();
	
}

Chunk* World::GetChunkAtCoords(IntVec2 const& chunkCoords)
{
	std::map<IntVec2, Chunk* >::iterator found = m_activeChunks.find(chunkCoords);
	if (found != m_activeChunks.end())
	{
		return found->second;
	}
	return nullptr;
}

void World::DeleteAllChunks()
{
	std::map<IntVec2, Chunk* > ::iterator chunkIter;
	for (chunkIter = m_activeChunks.begin(); chunkIter != m_activeChunks.end(); ++chunkIter)
	{
		Chunk* chunk = chunkIter->second;
		delete chunk;
	}
	m_activeChunks.clear();
	m_lightQueue.clear();
}

bool World::ActivateNearestIfAny()
{
	// Find the nearest IntVec2
	Vec2 playerPos2D = Vec2(m_theGame->m_player->m_position);
	int playerIntX = (int)m_theGame->m_player->m_position.x / CHUNK_BLOCK_NUM_OF_X;
	int playerIntY = (int)m_theGame->m_player->m_position.y / CHUNK_BLOCK_NUM_OF_Y;
	IntVec2 currentIn = IntVec2(playerIntX, playerIntY);
	int radius = (int)m_chunkActivationRange / CHUNK_BLOCK_NUM_OF_X;
	float nearestDist = 999999999999999.f;
	IntVec2 activatePos;
	bool isfound = false;
	for (int intX = playerIntX - radius; intX < playerIntX + radius; intX++ )
	{
		for (int intY = playerIntY - radius; intY < playerIntY + radius; intY++)
		{
			IntVec2 current = IntVec2(intX, intY);
			if (m_activeChunks.count(current) == 0)
			{
				bool Isgood = true;
				for (int i = 0; i < m_generatingChunks.size(); i++)
				{
					if (m_generatingChunks[i])
					{
						if (m_generatingChunks[i]->GetCoords() == current)
						{
							Isgood = false;
						}
					}
				}

				if (!Isgood)
				{
					continue;
				}

				bool hasToBeActivate = false;
				
				float cornerDist;
				AABB3 bounds;
				bounds.m_mins.x = (float)CHUNK_BLOCK_NUM_OF_X * (float)intX;
				bounds.m_mins.y = (float)CHUNK_BLOCK_NUM_OF_Y * (float)intY;
				bounds.m_mins.z = 0.f;

				bounds.m_maxs.x = (float)(CHUNK_BLOCK_NUM_OF_X) * (float)intX + (float)CHUNK_BLOCK_NUM_OF_X;
				bounds.m_maxs.y = (float)(CHUNK_BLOCK_NUM_OF_Y) * (float)intY + (float)CHUNK_BLOCK_NUM_OF_Y;
				bounds.m_maxs.z = (float)CHUNK_BLOCK_NUM_OF_Z;

				Vec2 concers[] =
				{
					Vec2(bounds.m_maxs.x, bounds.m_maxs.y),
					Vec2(bounds.m_maxs.x, bounds.m_mins.y),
					Vec2(bounds.m_mins.x, bounds.m_maxs.y),
					Vec2(bounds.m_mins.x, bounds.m_mins.y),
				};
				for (int concersIndex = 0; concersIndex < 4; concersIndex++)
				{
					cornerDist = GetDistance2D(playerPos2D, concers[concersIndex]);
					if (cornerDist < m_chunkActivationRange)
					{
						hasToBeActivate = true;;
					}
				}
				if (hasToBeActivate)
				{
					float dist = GetDistFromIntVec2(playerPos2D, current);
					if (nearestDist > dist)
					{
						isfound = true;
						activatePos = current;
						nearestDist = dist;
					}
				}
			}
		}
	}

	if (isfound)
	{
		Chunk* newChunk = new Chunk(this, activatePos);
		m_generatingChunks.push_back(newChunk);
		newChunk->GenerateChunk();
		return true;

		
	}

	return false;
}

bool World::DeactivateFurthestIfAny()
{
	// Find the furthest IntVec2
	Vec2 playerPos2D = Vec2(m_theGame->m_player->m_position);
	float furthestDist = 0.f;
	IntVec2 deactivatePos;
	bool isfound = false;

	for (auto const& pair : m_activeChunks)
	{
		if (pair.second != nullptr)
		{
			if ((pair.second)->ItHasToBeDeactivated(playerPos2D))
			{
				float dist = (pair.second)->GetDistFromPos(playerPos2D);
				if (furthestDist < dist)
				{
					isfound = true;
					deactivatePos = (pair.second)->GetCoords();
					furthestDist = dist;
				}
			}
		}
	}

	if (isfound)
	{
		if (m_activeChunks.count(deactivatePos) > 0)
		{
			Chunk* deactivateChunk = m_activeChunks[deactivatePos];
			UnHookNeighbors(deactivateChunk);
			IntVec2 key = deactivateChunk->GetCoords();
			m_activeChunks[key] = nullptr;
			m_activeChunks.erase(key);
			delete deactivateChunk;
		}
	}

	return false;
}

float World::GetDistFromIntVec2(Vec2 const& currentPos, IntVec2 const& targetCoords)
{
	AABB3 bounds;
	bounds.m_mins.x = (float)CHUNK_BLOCK_NUM_OF_X * (float)targetCoords.x;
	bounds.m_mins.y = (float)CHUNK_BLOCK_NUM_OF_Y * (float)targetCoords.y;
	bounds.m_mins.z = 0.f;

	bounds.m_maxs.x = (float)(CHUNK_BLOCK_NUM_OF_X) * (float)targetCoords.x + (float)CHUNK_BLOCK_NUM_OF_X;
	bounds.m_maxs.y = (float)(CHUNK_BLOCK_NUM_OF_Y) * (float)targetCoords.y + (float)CHUNK_BLOCK_NUM_OF_Y;
	bounds.m_maxs.z = (float)CHUNK_BLOCK_NUM_OF_Z;

	AABB2 box = AABB2(Vec2(bounds.m_mins), Vec2(bounds.m_maxs));
	Vec2 center = box.GetCenter();
	return GetDistance2D(currentPos, center);
}

void World::HookUpNeighbors(Chunk* chunk)
{
	IntVec2 north = chunk->GetNorth();
	IntVec2 south = chunk->GetSouth();
	IntVec2 east = chunk->GetEast();
	IntVec2 west = chunk->GetWest();

	if (m_activeChunks.count(north) > 0)
	{
		chunk->m_northNeighbor = m_activeChunks[north];
		m_activeChunks[north]->m_southNeighbor = chunk;
	}

	if (m_activeChunks.count(south) > 0)
	{
		chunk->m_southNeighbor = m_activeChunks[south];
		m_activeChunks[south]->m_northNeighbor = chunk;
	}

	if (m_activeChunks.count(east) > 0)
	{
		chunk->m_eastNeighbor = m_activeChunks[east];
		m_activeChunks[east]->m_westNeighbor = chunk;
	}

	if (m_activeChunks.count(west) > 0)
	{
		chunk->m_westNeighbor = m_activeChunks[west];
		m_activeChunks[west]->m_eastNeighbor = chunk;
	}

	for (int i = 0; i < m_generatingChunks.size(); i++)
	{
		if (m_generatingChunks[i])
		{
			if (m_generatingChunks[i]->GetCoords() == north)
			{
				chunk->m_northNeighbor = m_generatingChunks[i];
				m_generatingChunks[i]->m_southNeighbor = chunk;
			}
			if (m_generatingChunks[i]->GetCoords() == south)
			{
				chunk->m_southNeighbor = m_generatingChunks[i];
				m_generatingChunks[i]->m_northNeighbor = chunk;
			}
			if (m_generatingChunks[i]->GetCoords() == east)
			{
				chunk->m_eastNeighbor = m_generatingChunks[i];
				m_generatingChunks[i]->m_westNeighbor = chunk;
			}
			if (m_generatingChunks[i]->GetCoords() == west)
			{
				chunk->m_westNeighbor = m_generatingChunks[i];
				m_generatingChunks[i]->m_eastNeighbor = chunk;
			}
		}
	}


}

void World::UnHookNeighbors(Chunk*& deactivateChunk)
{
	IntVec2 north = deactivateChunk->GetNorth();
	IntVec2 south = deactivateChunk->GetSouth();
	IntVec2 east = deactivateChunk->GetEast();
	IntVec2 west = deactivateChunk->GetWest();

	if (m_activeChunks.count(north) > 0)
	{
		m_activeChunks[north]->m_southNeighbor = nullptr;
	}

	if (m_activeChunks.count(south) > 0)
	{
		m_activeChunks[south]->m_northNeighbor = nullptr;
	}

	if (m_activeChunks.count(east) > 0)
	{
		m_activeChunks[east]->m_westNeighbor = nullptr;
	}

	if (m_activeChunks.count(west) > 0)
	{
		m_activeChunks[west]->m_eastNeighbor = nullptr;
	}
}

void World::ProcessDirtyLighting()
{
	while (!m_lightQueue.empty())
	{
		ProcessNextDirtyLightBlock();
	}
}

void World::ProcessNextDirtyLightBlock()
{
	BlockIterator& blockIter = m_lightQueue.front();
	Block* block = blockIter.GetBlock();
	if (!block)
	{
		m_lightQueue.pop_front();
		return;
	}
	uint8_t defIndex = block->GetTypeIndex();
	BlockDef* def = BlockDef::GetDefByIndex(defIndex);


	Block* top = blockIter.GetTopNeighbor().GetBlock();
	Block* bott = blockIter.GetBottomNeighbor().GetBlock();
	Block* north = blockIter.GetNorthNeighbor().GetBlock();
	Block* south = blockIter.GetSouthNeighbor().GetBlock();
	Block* east = blockIter.GetEastNeighbor().GetBlock();
	Block* west = blockIter.GetWestNeighbor().GetBlock();

	block->SetIsBlockLightDirtyFalse();

	uint8_t theoreticallyCorrectIndoor = 0, theoreticallyCorrectOutdoor = 0;

	if (def->m_indoorLight != 0)
	{
		theoreticallyCorrectIndoor = def->m_indoorLight;
	}
	if (def->m_outdoorLight != 0)
	{
		theoreticallyCorrectOutdoor = def->m_outdoorLight;
	}
	
	if (!def->m_isOpaque)
	{
		uint8_t highestIndoorLevel = 1;
		uint8_t highestOutdoorLevel = 1;
		

		uint8_t topIndoorLevel, bottIndoorLevel, northIndoorLevel, southIndoorLevel, eastIndoorLevel, westIndoorLevel;
		uint8_t topOutdoorLevel, bottOutdoorLevel, northOutdoorLevel, southOutdoorLevel, eastOutdoorLevel, westOutdoorLevel;

		if (top)
		{
			topIndoorLevel = top->GetInDoorLightInfluence();
			if (topIndoorLevel > highestIndoorLevel)
			{
				highestIndoorLevel = topIndoorLevel;
			}
			topOutdoorLevel = top->GetOutDoorLightInfluence();
			if (topOutdoorLevel > highestOutdoorLevel)
			{
				highestOutdoorLevel = topOutdoorLevel;
			}
		}
		if (bott)
		{
			bottIndoorLevel = bott->GetInDoorLightInfluence();
			if (bottIndoorLevel > highestIndoorLevel)
			{
				highestIndoorLevel = bottIndoorLevel;
			}

			bottOutdoorLevel = bott->GetOutDoorLightInfluence();
			if (bottOutdoorLevel > highestOutdoorLevel)
			{
				highestOutdoorLevel = bottOutdoorLevel;
			}
		}

		if (north)
		{
			northIndoorLevel = north->GetInDoorLightInfluence();
			if (northIndoorLevel > highestIndoorLevel)
			{
				highestIndoorLevel = northIndoorLevel;
			}

			northOutdoorLevel = north->GetOutDoorLightInfluence();
			if (northOutdoorLevel > highestOutdoorLevel)
			{
				highestOutdoorLevel = northOutdoorLevel;
			}
		}

		if (south)
		{
			southIndoorLevel = south->GetInDoorLightInfluence();
			if (southIndoorLevel > highestIndoorLevel)
			{
				highestIndoorLevel = southIndoorLevel;
			}

			southOutdoorLevel = south->GetOutDoorLightInfluence();
			if (southOutdoorLevel > highestOutdoorLevel)
			{
				highestOutdoorLevel = southOutdoorLevel;
			}
		}

		if (east)
		{
			eastIndoorLevel = east->GetInDoorLightInfluence();
			if (eastIndoorLevel > highestIndoorLevel)
			{
				highestIndoorLevel = eastIndoorLevel;
			}

			eastOutdoorLevel = east->GetOutDoorLightInfluence();
			if (eastOutdoorLevel > highestOutdoorLevel)
			{
				highestOutdoorLevel = eastOutdoorLevel;
			}
		}

		if (west)
		{
			westIndoorLevel = west->GetInDoorLightInfluence();
			if (westIndoorLevel > highestIndoorLevel)
			{
				highestIndoorLevel = westIndoorLevel;
			}

			westOutdoorLevel = west->GetOutDoorLightInfluence();
			if (westOutdoorLevel > highestOutdoorLevel)
			{
				highestOutdoorLevel = westOutdoorLevel;
			}
		}

		theoreticallyCorrectIndoor = highestIndoorLevel - 1;
		theoreticallyCorrectOutdoor = highestOutdoorLevel - 1;
	}

	if (block->IsBlockSky())
	{
		theoreticallyCorrectOutdoor = 15;
	}

	int currentIndoor = block->GetInDoorLightInfluence();
	int currentOutdoor = block->GetOutDoorLightInfluence();

	if (currentOutdoor != theoreticallyCorrectOutdoor || currentIndoor != theoreticallyCorrectIndoor)
	{
		block->SetInDoorLightInfluence(theoreticallyCorrectIndoor);
		block->SetOutDoorLightInfluence(theoreticallyCorrectOutdoor);

		blockIter.m_owner->MarkDirty();

		if (blockIter.GetEastNeighbor().m_owner)
		{
			blockIter.GetEastNeighbor().m_owner->MarkDirty();

			MarkLightingDirtyIfNotOpaque(blockIter.GetEastNeighbor().GetBlock(), blockIter.GetEastNeighbor());
			
		}
		if (blockIter.GetWestNeighbor().m_owner)
		{
			blockIter.GetWestNeighbor().m_owner->MarkDirty();
			
			MarkLightingDirtyIfNotOpaque(blockIter.GetWestNeighbor().GetBlock(), blockIter.GetWestNeighbor());
		}

		if (blockIter.GetNorthNeighbor().m_owner)
		{
			blockIter.GetNorthNeighbor().m_owner->MarkDirty();
			
			MarkLightingDirtyIfNotOpaque(blockIter.GetNorthNeighbor().GetBlock(), blockIter.GetNorthNeighbor());
		}
		if (blockIter.GetSouthNeighbor().m_owner)
		{
			blockIter.GetSouthNeighbor().m_owner->MarkDirty();
			
			MarkLightingDirtyIfNotOpaque(blockIter.GetSouthNeighbor().GetBlock(), blockIter.GetSouthNeighbor());
		}

		if (blockIter.GetTopNeighbor().m_owner != nullptr)
		{
			blockIter.GetTopNeighbor().m_owner->MarkDirty();
		
			MarkLightingDirtyIfNotOpaque(blockIter.GetTopNeighbor().GetBlock(), blockIter.GetTopNeighbor());
		}
		if (blockIter.GetBottomNeighbor().m_owner)
		{
			blockIter.GetBottomNeighbor().m_owner->MarkDirty();
			
			MarkLightingDirtyIfNotOpaque(blockIter.GetBottomNeighbor().GetBlock(), blockIter.GetBottomNeighbor());
		}
	}

	m_lightQueue.pop_front();

}

void World::MarkLightingDirty(Chunk* chunk, int index)
{
	if (!chunk->GetBlockByIndex(index)->IsBlockLightDirty())
	{
		m_lightQueue.emplace_back(BlockIterator(chunk, index));
		chunk->GetBlockByIndex(index)->SetIsBlockLightDirtyTrue();
	}
}

IntVec2 World::GetChunkCoordinatesForPosition(Vec3 const& pos) const
{
	int x = RoundDownToInt(pos.x);
	int y = RoundDownToInt(pos.y);
	int chunkCoordsX = x >> CHUNK_SIZE_OF_X;
	int chunkCoordsY = y >> CHUNK_SIZE_OF_Y;
	return IntVec2(chunkCoordsX, chunkCoordsY);
}

Chunk* World::GetChunkForCoordinate(IntVec2 const& coordinate) const
{
	std::map<IntVec2, Chunk*>::const_iterator itr = m_activeChunks.find(coordinate);
	if (itr != m_activeChunks.end()) return itr->second;

	return nullptr;
}

GameRaycastResult3D World::RaycastVsBlocks(Vec3 const& startPos, Vec3 const& fwdNormal, float maxDist)
{
	IntVec2 const& chunkCoords = GetChunkCoordinatesForPosition(startPos);
	Chunk* chunk = GetChunkForCoordinate(chunkCoords);
	if (!chunk) return GameRaycastResult3D();
	int x = RoundDownToInt(startPos.x);
	int y = RoundDownToInt(startPos.y);
	int z = RoundDownToInt(startPos.z);
	int blockIndex = chunk->GetIndexForWorldCoords(IntVec3(x, y, z));
	BlockIterator currentBlockIterator(chunk, blockIndex);
	Block* currentBlock = currentBlockIterator.GetBlock();

	//if raycast inside a opaque block
	if (currentBlock && currentBlock->IsOpaque())
	{
		GameRaycastResult3D result(startPos, fwdNormal, maxDist, true, startPos, 0.f, -fwdNormal);
		result.m_impactBlock = currentBlockIterator;
		return result;
	}

	IntVec3 currentBlockCoords = currentBlockIterator.m_owner->GetWorldCoordsForIndex(currentBlockIterator.m_blockIndex);

	// x crossing 
	float fwdDistPerXCrossing = 0.f;
	int blockStepDirectionX = 0;
	float xAtFirstXCrossing = 0.f;
	float xDistToFirstXCrossing = 0.f;
	float fwdDistAtNextXCrossing = 0.f;

	if (fwdNormal.x == 0.f)
	{
		fwdDistAtNextXCrossing = maxDist + 1.f;
	}
	else
	{
		fwdDistPerXCrossing = 1.f / abs(fwdNormal.x);
		blockStepDirectionX = fwdNormal.x < 0.f ? -1 : 1;
		xAtFirstXCrossing = static_cast<float>(currentBlockCoords.x + (blockStepDirectionX + 1) / 2);
		xDistToFirstXCrossing = xAtFirstXCrossing - startPos.x;
		fwdDistAtNextXCrossing = fabsf(xDistToFirstXCrossing) * fwdDistPerXCrossing;
	}

	// y crossing
	float fwdDistPerYCrossing = 0.f;
	int blockStepDirectionY = 0;
	float yAtFirstYCrossing = 0.f;
	float yDistToFirstYCrossing = 0.f;
	float fwdDistAtNextYCrossing = 0.f;
	if (fwdNormal.y == 0.f)
	{
		fwdDistAtNextYCrossing = maxDist + 1.f;
	}
	else
	{
		fwdDistPerYCrossing = 1.f / abs(fwdNormal.y);
		blockStepDirectionY = fwdNormal.y < 0.f ? -1 : 1;
		yAtFirstYCrossing = static_cast<float>(currentBlockCoords.y + (blockStepDirectionY + 1) / 2);
		yDistToFirstYCrossing = yAtFirstYCrossing - startPos.y;
		fwdDistAtNextYCrossing = fabsf(yDistToFirstYCrossing) * fwdDistPerYCrossing;
	}

	// z crossing 
	float fwdDistPerZCrossing = 0.f;
	int blockStepDirectionZ = 0;
	float zAtFirstZCrossing = 0.f;
	float zDistToFirstZCrossing = 0.f;
	float fwdDistAtNextZCrossing = 0.f;
	if (fwdNormal.z == 0.f)
	{
		fwdDistAtNextZCrossing = maxDist + 1.f;
	}
	else
	{
		fwdDistPerZCrossing = 1.f / abs(fwdNormal.z);
		blockStepDirectionZ = fwdNormal.z < 0.f ? -1 : 1;
		zAtFirstZCrossing = static_cast<float>(currentBlockCoords.z + (blockStepDirectionZ + 1) / 2);
		zDistToFirstZCrossing = zAtFirstZCrossing - startPos.z;
		fwdDistAtNextZCrossing = fabsf(zDistToFirstZCrossing) * fwdDistPerZCrossing;
	}

	for (; ;)
	{
		if (fwdDistAtNextXCrossing >= maxDist && fwdDistAtNextYCrossing >= maxDist && fwdDistAtNextZCrossing >= maxDist)
		{
			Vec3 endPosition = startPos + maxDist * fwdNormal;
			return GameRaycastResult3D(startPos, fwdNormal, maxDist, false, endPosition, maxDist, fwdNormal);
		}

		if (fwdDistAtNextXCrossing <= fwdDistAtNextYCrossing && fwdDistAtNextXCrossing <= fwdDistAtNextZCrossing)  // x crossing
		{
			if (blockStepDirectionX > 0)
			{
				currentBlockIterator = currentBlockIterator.GetEastNeighbor();
			}
			else
			{
				currentBlockIterator = currentBlockIterator.GetWestNeighbor();
			}

			currentBlock = currentBlockIterator.GetBlock();
			if (currentBlock && currentBlock->IsOpaque())
			{
				Vec3 impactPosition = startPos + fwdDistAtNextXCrossing * fwdNormal;
				GameRaycastResult3D result(startPos, fwdNormal, maxDist, true, impactPosition, fwdDistAtNextXCrossing, Vec3(static_cast<float>(-blockStepDirectionX), 0.f, 0.f));
				result.m_impactBlock = currentBlockIterator;
				return result;
			}
			else
			{
				fwdDistAtNextXCrossing += fwdDistPerXCrossing;
			}
		}
		else if (fwdDistAtNextYCrossing <= fwdDistAtNextXCrossing && fwdDistAtNextYCrossing <= fwdDistAtNextZCrossing) // y crossing
		{
			if (blockStepDirectionY > 0)
			{
				currentBlockIterator = currentBlockIterator.GetNorthNeighbor();
			}
			else
			{
				currentBlockIterator = currentBlockIterator.GetSouthNeighbor();
			}

			currentBlock = currentBlockIterator.GetBlock();
			if (currentBlock && currentBlock->IsOpaque())
			{
				Vec3 impactPosition = startPos + fwdDistAtNextYCrossing * fwdNormal;
				GameRaycastResult3D result(startPos, fwdNormal, maxDist, true, impactPosition, fwdDistAtNextYCrossing, Vec3(0.f, static_cast<float>(-blockStepDirectionY), 0.f));
				result.m_impactBlock = currentBlockIterator;
				return result;
			}
			else
			{
				fwdDistAtNextYCrossing += fwdDistPerYCrossing;
			}
		}
		else // z crossing
		{
			if (blockStepDirectionZ > 0)
			{
				currentBlockIterator = currentBlockIterator.GetTopNeighbor();
			}
			else
			{
				currentBlockIterator = currentBlockIterator.GetBottomNeighbor();
			}

			currentBlock = currentBlockIterator.GetBlock();
			if (currentBlock && currentBlock->IsOpaque())
			{
				Vec3 impactPosition = startPos + fwdDistAtNextZCrossing * fwdNormal;
				GameRaycastResult3D result(startPos, fwdNormal, maxDist, true, impactPosition, fwdDistAtNextZCrossing, Vec3(0.f, 0.f, static_cast<float>(-blockStepDirectionZ)));
				result.m_impactBlock = currentBlockIterator;
				return result;
			}
			else
			{
				fwdDistAtNextZCrossing += fwdDistPerZCrossing;
			}
		}
	}
}

void World::RenderCurrentFaceOfBlock() const
{
	if (!m_currentPlayerRaycastResult.m_didImpact)
	{
		return;
	}

	Vec3 facePointA;
	Vec3 facePointB;
	Vec3 facePointC;
	Vec3 facePointD;

	float thickness = 0.01f;

	Vec3 blockPosCenter = m_currentPlayerRaycastResult.m_impactBlock.GetWorldCenter();
	Vec3 impactNormal = m_currentPlayerRaycastResult.m_impactNormal;

	if (impactNormal == Vec3(1.f, 0.f, 0.f)) //east
	{
		facePointA = Vec3(blockPosCenter.x + 0.5f, blockPosCenter.y - 0.5f, blockPosCenter.z - 0.5f);
		facePointB = Vec3(blockPosCenter.x + 0.5f, blockPosCenter.y - 0.5f, blockPosCenter.z + 0.5f);
		facePointC = Vec3(blockPosCenter.x + 0.5f, blockPosCenter.y + 0.5f, blockPosCenter.z - 0.5f);
		facePointD = Vec3(blockPosCenter.x + 0.5f, blockPosCenter.y + 0.5f, blockPosCenter.z + 0.5f);
	}
	else if (impactNormal == Vec3(-1.f, 0.f, 0.f)) // west
	{
		facePointA = Vec3(blockPosCenter.x - 0.5f, blockPosCenter.y - 0.5f, blockPosCenter.z - 0.5f);
		facePointB = Vec3(blockPosCenter.x - 0.5f, blockPosCenter.y - 0.5f, blockPosCenter.z + 0.5f);
		facePointC = Vec3(blockPosCenter.x - 0.5f, blockPosCenter.y + 0.5f, blockPosCenter.z - 0.5f);
		facePointD = Vec3(blockPosCenter.x - 0.5f, blockPosCenter.y + 0.5f, blockPosCenter.z + 0.5f);
	}
	else if (impactNormal == Vec3(0.f, 1.f, 0.f)) // north
	{
		facePointA = Vec3(blockPosCenter.x - 0.5f, blockPosCenter.y + 0.5f, blockPosCenter.z - 0.5f);
		facePointB = Vec3(blockPosCenter.x - 0.5f, blockPosCenter.y + 0.5f, blockPosCenter.z + 0.5f);
		facePointC = Vec3(blockPosCenter.x + 0.5f, blockPosCenter.y + 0.5f, blockPosCenter.z - 0.5f);
		facePointD = Vec3(blockPosCenter.x + 0.5f, blockPosCenter.y + 0.5f, blockPosCenter.z + 0.5f);
	}
	else if (impactNormal == Vec3(0.f, -1.f, 0.f)) // south
	{
		facePointA = Vec3(blockPosCenter.x - 0.5f, blockPosCenter.y - 0.5f, blockPosCenter.z - 0.5f);
		facePointB = Vec3(blockPosCenter.x - 0.5f, blockPosCenter.y - 0.5f, blockPosCenter.z + 0.5f);
		facePointC = Vec3(blockPosCenter.x + 0.5f, blockPosCenter.y - 0.5f, blockPosCenter.z - 0.5f);
		facePointD = Vec3(blockPosCenter.x + 0.5f, blockPosCenter.y - 0.5f, blockPosCenter.z + 0.5f);
	}
	else if (impactNormal == Vec3(0.f, 0.f, 1.f)) // top
	{
		facePointA = Vec3(blockPosCenter.x - 0.5f, blockPosCenter.y - 0.5f, blockPosCenter.z + 0.5f);
		facePointB = Vec3(blockPosCenter.x - 0.5f, blockPosCenter.y + 0.5f, blockPosCenter.z + 0.5f);
		facePointC = Vec3(blockPosCenter.x + 0.5f, blockPosCenter.y - 0.5f, blockPosCenter.z + 0.5f);
		facePointD = Vec3(blockPosCenter.x + 0.5f, blockPosCenter.y + 0.5f, blockPosCenter.z + 0.5f);
	}
	else if (impactNormal == Vec3(0.f, 0.f, -1.f)) // bott
	{
		facePointA = Vec3(blockPosCenter.x - 0.5f, blockPosCenter.y - 0.5f, blockPosCenter.z - 0.5f);
		facePointB = Vec3(blockPosCenter.x - 0.5f, blockPosCenter.y + 0.5f, blockPosCenter.z - 0.5f);
		facePointC = Vec3(blockPosCenter.x + 0.5f, blockPosCenter.y - 0.5f, blockPosCenter.z - 0.5f);
		facePointD = Vec3(blockPosCenter.x + 0.5f, blockPosCenter.y + 0.5f, blockPosCenter.z - 0.5f);
	}


	std::vector<Vertex_PCU> faceVerts;
	AddVertsForLineSegment3D(faceVerts, facePointA, facePointB, thickness, Rgba8::MAGENTA);
	AddVertsForLineSegment3D(faceVerts, facePointB, facePointD, thickness, Rgba8::MAGENTA);
	AddVertsForLineSegment3D(faceVerts, facePointC, facePointD, thickness, Rgba8::MAGENTA);
	AddVertsForLineSegment3D(faceVerts, facePointC, facePointA, thickness, Rgba8::MAGENTA);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->DrawVertexArray((int)faceVerts.size(), faceVerts.data());
}

void World::DigRaycastResult()
{
	if (m_currentPlayerRaycastResult.m_didImpact)
	{
		BlockIterator& blockIter = m_currentPlayerRaycastResult.m_impactBlock;
		Block* block = blockIter.GetBlock();
		
		block->ChangeTypeTo(0);
		block->SetIsBlockLightDirtyTrue();
		m_lightQueue.emplace_back(blockIter);
		blockIter.m_owner->MarkDirty();
		blockIter.m_owner->m_needsSaving = true;
		if (blockIter.GetTopNeighbor().GetBlock()->IsBlockSky())
		{
			block->SetIsBlockSkyTrue();
			BlockIterator current = blockIter;
			Block* currentBlock = current.GetBlock();
			while (currentBlock->IsBlockSky())
			{
				BlockIterator bottOne = current.GetBottomNeighbor();
				Block* bott = bottOne.GetBlock();
				bott->SetIsBlockSkyTrue();
				bott->SetIsBlockLightDirtyTrue();
				m_lightQueue.emplace_back(bottOne);
				if (bott->IsOpaque())
				{
					return;
				}
			}
		}
	}

}

void World::PutRaycastResult(uint8_t type)
{
	if (m_currentPlayerRaycastResult.m_didImpact)
	{
		BlockIterator& blockIter = m_currentPlayerRaycastResult.m_impactBlock;
		Block* block = blockIter.GetBlock();
		Vec3 impactNormal = m_currentPlayerRaycastResult.m_impactNormal;
		Block* targetBlock = nullptr;
		BlockIterator targetBlockIter;

		if (impactNormal == Vec3(1.f, 0.f, 0.f)) //east
		{
			targetBlockIter = blockIter.GetEastNeighbor();
			targetBlock = targetBlockIter.GetBlock();
			targetBlock->ChangeTypeTo(type);
			targetBlock->SetIsBlockLightDirtyTrue();
			m_lightQueue.emplace_back(targetBlockIter);
		}
		else if (impactNormal == Vec3(-1.f, 0.f, 0.f)) // west
		{
			targetBlockIter = blockIter.GetWestNeighbor();
			targetBlock = targetBlockIter.GetBlock();
			targetBlock->ChangeTypeTo(type);
			targetBlock->SetIsBlockLightDirtyTrue();
			m_lightQueue.emplace_back(targetBlockIter);
		}
		else if (impactNormal == Vec3(0.f, 1.f, 0.f)) // north
		{
		    targetBlockIter = blockIter.GetNorthNeighbor();
			targetBlock = targetBlockIter.GetBlock();
			targetBlock->ChangeTypeTo(type);
			targetBlock->SetIsBlockLightDirtyTrue();
			m_lightQueue.emplace_back(targetBlockIter);
		}
		else if (impactNormal == Vec3(0.f, -1.f, 0.f)) // south
		{
			targetBlockIter = blockIter.GetSouthNeighbor();
			targetBlock = targetBlockIter.GetBlock();
			targetBlock->ChangeTypeTo(type);
			targetBlock->SetIsBlockLightDirtyTrue();
			m_lightQueue.emplace_back(targetBlockIter);
		}
		else if (impactNormal == Vec3(0.f, 0.f, 1.f)) // top
		{
			targetBlockIter = blockIter.GetTopNeighbor();
			targetBlock = targetBlockIter.GetBlock();
			targetBlock->ChangeTypeTo(type);
			targetBlock->SetIsBlockLightDirtyTrue();
			m_lightQueue.emplace_back(targetBlockIter);
		}
		else if (impactNormal == Vec3(0.f, 0.f, -1.f)) // bott
		{
			targetBlockIter = blockIter.GetBottomNeighbor();
			targetBlock = targetBlockIter.GetBlock();
			targetBlock->ChangeTypeTo(type);
			targetBlock->SetIsBlockLightDirtyTrue();
			m_lightQueue.emplace_back(targetBlockIter);
		}
		targetBlockIter.m_owner->MarkDirty();
		targetBlockIter.m_owner->m_needsSaving = true;

		if (block->IsBlockSky() && targetBlock->IsOpaque())
		{
			block->SetIsBlockSkyFalse();
			BlockIterator current = blockIter;
			Block* currentBlock = current.GetBlock();
			while (currentBlock->IsBlockSky())
			{
				BlockIterator bottOne = current.GetBottomNeighbor();
				Block* bott = bottOne.GetBlock();
				bott->SetIsBlockSkyFalse();
				bott->SetIsBlockLightDirtyTrue();
				m_lightQueue.emplace_back(bottOne);
				if (bott->IsOpaque())
				{
					return;
				}
			}
		}
		
	}

}

unsigned int World::GetWorldSeed() const
{
	return m_theGame->m_app->m_setting.WorldSeed;
}

void World::UndirtyAllBlocksInChunk(Chunk* chunk)
{
	for (auto iter = m_lightQueue.begin(); iter != m_lightQueue.end(); ++iter)
	{
		if ((iter->m_owner) == chunk)
		{
			m_lightQueue.erase(iter);
		}
	}
}



void World::UpdateCurrentRaycastResult()
{
	Player* player = m_theGame->m_player;
	Vec3 pos = player->m_position;
	Vec3 dir = player->m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D().GetNormalized();
	float maxDist = m_theGame->m_app->m_setting.raycastMaxDist;

	if (g_theInput->WasKeyJustPressed('R'))
	{
		m_currentLockedData.pos = pos;
		m_currentLockedData.dir = dir;
		m_currentLockedData.maxDist = maxDist;
		m_isRaycastLocked = !m_isRaycastLocked;
		FireEvent("debugrenderclear");
		if (!m_isRaycastLocked)
		{
			m_isAddingDebugRender = false;
		}
	}

	if (pos.z > CHUNK_BLOCK_NUM_OF_Z || pos.z < 0.f)
	{
		return;
	}

	if (!m_isRaycastLocked)
	{
		m_currentPlayerRaycastResult = RaycastVsBlocks(pos, dir, maxDist);
	}
	else
	{
		m_currentPlayerRaycastResult = RaycastVsBlocks(m_currentLockedData.pos, m_currentLockedData.dir, m_currentLockedData.maxDist);
	}

	if (m_isRaycastLocked && !m_isAddingDebugRender)
	{
		Vec3 end = m_currentLockedData.pos + m_currentLockedData.dir * m_currentLockedData.maxDist;
		Vec3 normalEnd = m_currentPlayerRaycastResult.m_impactPosition + m_currentPlayerRaycastResult.m_impactNormal * 1.5f;
		DebugAddWorldLine(m_currentPlayerRaycastResult.m_impactPosition, normalEnd, 0.006f, 9999999999.f, Rgba8::LIGHTBLUE);
		DebugAddWorldLine(m_currentPlayerRaycastResult.m_rayStartPosition, end, 0.006f, 9999999999.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
		DebugAddWorldLine(m_currentPlayerRaycastResult.m_rayStartPosition, m_currentPlayerRaycastResult.m_impactPosition, 0.007f, 9999999999.f, Rgba8::RED, Rgba8::RED);
		m_isAddingDebugRender = true;
	}
}


void World::DebugDrawCavesForAllChunks() const
{
	std::vector<Vertex_PCU> caveDebugVerts;
	std::map<IntVec2, Chunk*>::const_iterator chunkIter;
	//to-do
}

bool World::CheckIfWorldFolderExist()
{
	worldFolder = Stringf("Saves/World_%u", GetWorldSeed());
	DWORD fileAttributes = GetFileAttributesA(worldFolder.c_str());

	if (fileAttributes == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	return (fileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

void World::ForceCreateWorldFolder()
{
	if (CreateDirectoryA(worldFolder.c_str(), NULL))
	{
		
	}
	else 
	{
		ERROR_RECOVERABLE("Creating File Folder fail!");
	}
}

void World::RetrieveCompletedJobs()
{
	Job* completedJob = g_theJobSystem->RetrieveCompletedJobs();
	while (completedJob)
	{
		if (ChunkGenerationJob* job = dynamic_cast<ChunkGenerationJob*>(completedJob))
		{
			Chunk* chunk = job->m_chunk;
			m_activeChunks.insert(std::make_pair(chunk->GetCoords(), chunk));
			HookUpNeighbors(chunk);
			chunk->MarkNeighborDirtyIfTouchingNonOpaqueBoundary();
			chunk->DescendEachColumnFlaging_SKY();
			chunk->HorizontalSpread();
			chunk->MarkAllEmitsLightBlockDirty();
			for (int i = 0; i < m_generatingChunks.size(); i++)
			{
				if (m_generatingChunks[i] == chunk)
				{
					m_generatingChunks[i] = nullptr;
				}
			}
			chunk->m_chunkState = ChunkState::ACTIVE;
		}

		delete completedJob;
		completedJob = g_theJobSystem->RetrieveCompletedJobs();
	}
}

void World::MarkLightingDirtyIfNotOpaque(Block* block, BlockIterator const& iter)
{
	if (!block)
	{
		return;
	}

	if (block->IsBlockLightDirty())
	{
		return;
	}
	if (block->GetTypeIndex() > BlockDef::s_blockDefinitions.size())
	{
		return;
	}
	if (!block->IsOpaque())
	{
		m_lightQueue.emplace_back(iter);
		block->SetIsBlockLightDirtyTrue();
		return;
	}
}


