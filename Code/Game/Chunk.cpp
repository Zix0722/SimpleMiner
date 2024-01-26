#include <fstream>
#include "Chunk.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/World.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Game/BlockDef.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "ThirdParty/Squirrel/SmoothNoise.hpp"
#include "Engine/Math/Easing.hpp"
#include "BlockTemplate.hpp"
#include "ThirdParty/Squirrel/RawNoise.hpp"
#include "GameJobs.hpp"

CaveInfo::CaveInfo(IntVec2 const &coords)
	:m_startChunkCoords(coords)
{

}

Chunk::Chunk(World* world, IntVec2 coords)
	:m_theWorld(world)
	,m_chunkCoords(coords)
{
	m_chunkState = ChunkState::CONSTRUCTING;
	m_bounds.m_mins.x = (float)CHUNK_BLOCK_NUM_OF_X * (float)m_chunkCoords.x;
	m_bounds.m_mins.y = (float)CHUNK_BLOCK_NUM_OF_Y * (float)m_chunkCoords.y;
	m_bounds.m_mins.z = 0.f;

	m_bounds.m_maxs.x = (float)(CHUNK_BLOCK_NUM_OF_X) * (float)m_chunkCoords.x + (float)CHUNK_BLOCK_NUM_OF_X;
	m_bounds.m_maxs.y = (float)(CHUNK_BLOCK_NUM_OF_Y) * (float)m_chunkCoords.y + (float)CHUNK_BLOCK_NUM_OF_Y;
	m_bounds.m_maxs.z = (float)CHUNK_BLOCK_NUM_OF_Z;

	m_blocks = new Block[CHUNK_BLOCK_TOTAL];


	

}

Chunk::~Chunk()
{
	if (m_needsSaving)
	{
		SaveData();
	}
	
	delete m_gpuMesh;
	m_gpuMesh = nullptr;
	m_cpuMesh.clear();
	delete[] m_blocks;
	m_blocks = nullptr;
}

void Chunk::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	RefreshCPUMesh();
}

void Chunk::Render() const
{
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->BindShader(m_theWorld->m_theGame->m_shader);
	g_theRenderer->BindTexture(&m_theWorld->m_worldSpriteSheet->GetTexture());
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->DrawVertexArray(m_gpuMesh, (int)m_cpuMesh.size());
	// Debug Draw
	Game*& theGame = m_theWorld->m_theGame;
	if (theGame->g_DebugMo)
	{
		std::vector<Vertex_PCU> debugVerts;

		AddVertsForAABB3(debugVerts, m_bounds);

		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_BACK);
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->DrawVertexArray((int)debugVerts.size(), debugVerts.data());
	}
}

void Chunk::InitializedBlocks()
{
	m_chunkState = ChunkState::ACTIVATING_GENERRATING;
	RandomNumberGenerator localRNG = RandomNumberGenerator();
	localRNG.m_seed = m_theWorld->GetWorldSeed() + m_chunkCoords.x * (m_chunkCoords.y * 10);
	
	constexpr int MAX_VERTS = CHUNK_BLOCK_TOTAL * 6 * 2 * 3;
	m_cpuMesh.reserve(MAX_VERTS);

	int maxIceDepth = 6;
	int maxsandDepth = 4;

	Vec3 worldOriginalPos = m_bounds.m_mins;

	Vec3 bLB = m_bounds.m_mins;

	uint8_t air = 0;
	uint8_t stone = 1;
	uint8_t dirt = 2;
	uint8_t grass = 3;
	uint8_t coal =  4;
	uint8_t iron = 5;
	uint8_t water = 8;
	uint8_t sand = 12;
	uint8_t ice = 13;
	uint8_t snow = 20;


	std::map<IntVec2, float> treenessNoise;
	std::map<IntVec2, float> houseNoise;

	int const NUM_BLOCKS_A_CHUNK = CHUNK_BLOCK_NUM_OF_X * CHUNK_BLOCK_NUM_OF_Y;
	int Heights[NUM_BLOCKS_A_CHUNK];
	float terrainNoise[NUM_BLOCKS_A_CHUNK];
	float humiditys[NUM_BLOCKS_A_CHUNK];
	float temperatures[NUM_BLOCKS_A_CHUNK];
	float hilliness[NUM_BLOCKS_A_CHUNK];
	float oceanness[NUM_BLOCKS_A_CHUNK];

	int chunkGlobalX = m_chunkCoords.x << CHUNK_SIZE_OF_X;
	int chunkGlobalY = m_chunkCoords.y << CHUNK_SIZE_OF_Y;
	int worldSeed = m_theWorld->m_theGame->m_app->m_setting.WorldSeed;

	int NoiseExtend = 2 + TREE_RADIUS;
	int HouseNoiseExtend = 2 + HOUSE_RADIUS;
	int caveWorldSeed = worldSeed + 7;
	unsigned int houseWorldSeed = worldSeed + 8;

	for (int y = -NoiseExtend; y < CHUNK_BLOCK_NUM_OF_Y + NoiseExtend; y++)
	{
		for (int x = -NoiseExtend; x < CHUNK_BLOCK_NUM_OF_X + NoiseExtend; x++)
		{
			float worldX = (float)(chunkGlobalX + x);
			float worldY = (float)(chunkGlobalY + y);
			float treeDensity = 0.5f + 0.5f * Compute2dPerlinNoise(worldX, worldY, 100.f, 2, 0.5f, 10.0f, true, worldSeed + 5);
			treenessNoise[IntVec2(x, y)] = 0.5f + 0.5f * Compute2dPerlinNoise(worldX, worldY, 250.f, 5, treeDensity, 2.0f, true, worldSeed + 6);
		}
	}

	for (int y = -HouseNoiseExtend; y < CHUNK_BLOCK_NUM_OF_Y + HouseNoiseExtend; y++)
	{
		for (int x = -HouseNoiseExtend; x < CHUNK_BLOCK_NUM_OF_X + HouseNoiseExtend; x++)
		{
			IntVec2 currentCoords = m_chunkCoords + IntVec2(x, y);
			houseNoise[currentCoords] = 0.5f + 0.5f * Compute2dPerlinNoise(static_cast<float>(currentCoords.x), static_cast<float>(currentCoords.y), 20.f, 2, 0.5f, 2.0f, true, houseWorldSeed);
		}
	}

	for (int xyIndex = 0; xyIndex < NUM_BLOCKS_A_CHUNK; ++xyIndex)
	{
		int x = xyIndex % CHUNK_BLOCK_NUM_OF_X;
		int y = xyIndex / CHUNK_BLOCK_NUM_OF_X;
		int xCoords = (int)(CHUNK_BLOCK_NUM_OF_X * m_chunkCoords.x + 1.f * (float)x);
		int yCoords = (int)(CHUNK_BLOCK_NUM_OF_Y * m_chunkCoords.y + 1.f * (float)y);

		terrainNoise[xyIndex] = Compute2dPerlinNoise(float(xCoords), float(yCoords), 200.f, 5, 0.5f, 2.0f, worldSeed);	
		humiditys[xyIndex] = 0.5f + 0.5f * Compute2dPerlinNoise(float(xCoords), float(yCoords), 800.f, 3, 2.75f, 2.0f, true, worldSeed + 1);
		temperatures[xyIndex] = 0.5f + 0.5f * Compute2dPerlinNoise(float(xCoords), float(yCoords), 200.f, 3, 0.1f, 30.0f, true, worldSeed + 2);
		temperatures[xyIndex] += 0.003f * Get2dNoiseZeroToOne(xCoords, yCoords, 21);
		hilliness[xyIndex] = SmoothStep3(0.5f + 0.5f * Compute2dPerlinNoise(float(xCoords), float(yCoords), 1000.f, 9, 2.0f, 0.5f, true, worldSeed + 3));
		terrainNoise[xyIndex] *= hilliness[xyIndex];
		oceanness[xyIndex] = SmoothStart3(Compute2dPerlinNoise(float(xCoords), float(yCoords), 1200.f, 2, 0.5f, 0.5f, true, worldSeed + 4));
		float terrainRawHeight = static_cast<float>((SEA_LEVEL - RIVER_WIDTH) + 50.f * fabsf((float)terrainNoise[xyIndex]));
		int terrainHeight = static_cast<int>(RangeMapClamped(oceanness[xyIndex], 0.f, 0.5f, terrainRawHeight, MAX_DEPTH_SEA));
		Heights[xyIndex] = terrainHeight;
	}
	AddCaves(localRNG, caveWorldSeed);

	for (int blockIndex = 0; blockIndex < CHUNK_BLOCK_TOTAL; ++blockIndex)
	{
		int x = blockIndex & (CHUNK_BLOCK_NUM_OF_X - 1);
		int y = (blockIndex >> CHUNK_SIZE_OF_X) & (CHUNK_BLOCK_NUM_OF_X - 1);
		int z = blockIndex >> (CHUNK_SIZE_OF_X + CHUNK_SIZE_OF_Y);

		BlockIterator currentBlock = BlockIterator(this, blockIndex);

		Vec3 offSet = Vec3(1.f * x, 1.f * y, 1.f * z);
		Vec3 blockBLB = bLB + offSet;

		uint8_t typeIndex = 0;
		int randHeightDirt = localRNG.RollRandomIntInRange(3, 4);
		int xyIndex = x + y * CHUNK_BLOCK_NUM_OF_X;
		int terrainHeightZ = Heights[xyIndex];
		float humidity = humiditys[xyIndex];
		float temperature = temperatures[xyIndex];

		int iceDepth = RoundDownToInt(RangeMapClamped(temperature, 0.f, 0.4f, float(maxIceDepth), 0.f));
		int sandDepth = RoundDownToInt(RangeMapClamped(temperature, 0.f, 0.4f, float(maxsandDepth), 0.f));

		if (blockBLB.z == terrainHeightZ)
		{
			if (temperature < TEMP_THRESHOLD)
			{
				typeIndex = snow;
			}
			
			else if (humidity < HUMIDITY_THRESHOLD)
			{
				typeIndex = sand;
			}
			else
			{
				typeIndex = grass;
			}
		}
		else if(blockBLB.z + randHeightDirt >= terrainHeightZ && blockBLB.z < terrainHeightZ)
		{
			if (humidity < HUMIDITY_THRESHOLD)
			{
				if (blockBLB.z + randHeightDirt > terrainHeightZ - sandDepth)
				{
					typeIndex = sand;
				}
				else
				{
					typeIndex = dirt;
				}
			}
			else
			{
				typeIndex = dirt;
			}
		}
		else if(blockBLB.z < terrainHeightZ)
		{
			float randValue = localRNG.RollRandomFloatZeroToOne();
			if (randValue >= 0.f && randValue < 0.05f)
			{
				typeIndex = coal;
			}
			else if (randValue >= 0.05f && randValue < 0.07f)
			{
				typeIndex = iron;
			}
			else if (randValue >= 0.07f && randValue < 0.075f)
			{
				typeIndex = 6;
			}
			else if (randValue >= 0.075f && randValue < 0.076f)
			{
				typeIndex = 7;
			}
			else
			{
				typeIndex = stone;
			}
		}
		if (z <= SEA_LEVEL && typeIndex == air)
		{
			
			if (temperature < TEMP_THRESHOLD)
			{
				if (z > terrainHeightZ - iceDepth)
				{
					typeIndex = ice;
				}
				else
				{
					typeIndex = water;
				}
			}
			else
			{
				typeIndex = water;
			}
			
		}
		if (z == SEA_LEVEL && typeIndex == grass && humiditys[xyIndex] < BEACH_THRESHOLD)
		{
			typeIndex = sand;
		}

 		for (int caveIndex = 0; caveIndex < m_caves.size(); caveIndex++)
 		{
 			CaveCapsule& cave = m_caves[caveIndex];
			Vec3 start = cave.start;
			Vec3 end = cave.end;
			Vec3 currentMid = currentBlock.GetWorldCenter();

			if (IsPointInsideCapsule3D(currentMid, start, end, CAVE_CAPSULE_RADIUS))
			{
				typeIndex = air;
			}

 			
 
 		}

		m_blocks[blockIndex] = Block(typeIndex);

	}
	

	for (int y = -TREE_RADIUS; y < CHUNK_BLOCK_NUM_OF_Y + TREE_RADIUS; y++)
	{
		for (int x = -TREE_RADIUS; x < CHUNK_BLOCK_NUM_OF_X + TREE_RADIUS; x++)
		{
			IntVec2 currentCoords(x, y);
			if (IsLocalCoordsTreenessLocalMax(currentCoords, treenessNoise))
			{
				GenerateTreeAt(currentCoords);
			}
		}
	}

	if (IsCoordsVillageLocalMax(houseNoise))
	{
		GenerateHouseAt();
	}

	m_isVertexDirty = true;

	m_chunkState = ChunkState::ACTIVATING_GENERATE_COMPLETE;
}

void Chunk::RefreshCPUMesh()
{
	if (!m_isVertexDirty || m_northNeighbor == nullptr || m_eastNeighbor == nullptr || m_southNeighbor == nullptr || m_westNeighbor == nullptr)
	{
		return;
	}
	m_cpuMesh.clear();
	delete m_gpuMesh;

	SpriteSheet*& spriteSheet = m_theWorld->m_worldSpriteSheet;

	Vec3 bLB = m_bounds.m_mins;
	Vec3 bLF = bLB + Vec3(0.f, 1.f, 0.f);
	Vec3 bRB = bLB + Vec3(1.f, 0.f, 0.f);
	Vec3 bRF = bLB + Vec3(1.f, 1.f, 0.f);

	Vec3 tLB = bLB + Vec3(0.f, 0.f, 1.f);
	Vec3 tLF = bLF + Vec3(0.f, 0.f, 1.f);
	Vec3 tRB = bRB + Vec3(0.f, 0.f, 1.f);
	Vec3 tRF = bRF + Vec3(0.f, 0.f, 1.f);

	for (int blockIndex = 0; blockIndex < CHUNK_BLOCK_TOTAL; ++blockIndex)
	{
		BlockIterator blockIter = BlockIterator(this, blockIndex);

		unsigned char redChannelVal = 255;
		unsigned char greenChannelVal = 255;
		unsigned char blueChannelVal = 127;

		int x = blockIndex & (CHUNK_BLOCK_NUM_OF_X - 1);
		int y = (blockIndex >> CHUNK_SIZE_OF_X) & (CHUNK_BLOCK_NUM_OF_X - 1);
		int z = blockIndex >> (CHUNK_SIZE_OF_X + CHUNK_SIZE_OF_Y);

		Vec3 offSet = Vec3(1.f * x, 1.f * y, 1.f * z);
		Vec3 blockBLB = bLB + offSet;
		Vec3 blockBLF = bLF + offSet;
		Vec3 blockBRB = bRB + offSet;
		Vec3 blockBRF = bRF + offSet;

		Vec3 blockTLB = tLB + offSet;
		Vec3 blockTLF = tLF + offSet;
		Vec3 blockTRB = tRB + offSet;
		Vec3 blockTRF = tRF + offSet;
		
		uint8_t typeIndex = m_blocks[blockIndex].GetTypeIndex();
		

		if (typeIndex == 0)
		{
			continue;
		}

		int topIndex, bottIndex, sideIndex;
		BlockDef* currentDef = BlockDef::GetDefByIndex(typeIndex);

		topIndex = currentDef->m_UVCoords_Top.y * 64 + currentDef->m_UVCoords_Top.x;
		bottIndex = currentDef->m_UVCoords_Bottom.y * 64 + currentDef->m_UVCoords_Bottom.x;
		sideIndex = currentDef->m_UVCoords_Sides.y * 64 + currentDef->m_UVCoords_Sides.x;

		if (m_theWorld->m_theGame->m_app->m_setting.hiddenSurfaceRemoval)
		{
			if (z < CHUNK_BLOCK_NUM_OF_Z - 1)
			{
				Block* topBlock = blockIter.GetTopNeighbor().GetBlock();
				uint8_t topType = topBlock->GetTypeIndex();

				uint8_t indoorVal = topBlock->GetInDoorLightInfluence();
				uint8_t outdoorVal = topBlock->GetOutDoorLightInfluence();

				redChannelVal = (unsigned char)RangeMapClamped((float)outdoorVal, 0.f, 15.f, 50.f, 255.f);
				greenChannelVal = (unsigned char)RangeMapClamped((float)indoorVal, 0.f, 15.f, 50.f, 255.f);

				bool isOpaque = BlockDef::GetDefByIndex(topType)->m_isOpaque;
				if (!isOpaque)
				{
					AABB2 topUV = spriteSheet->GetSpriteUVs(topIndex);
					AddVertsForQuad3D(m_cpuMesh, blockTLB, blockTRB, blockTRF, blockTLF, Rgba8(redChannelVal, greenChannelVal, blueChannelVal), topUV);
				}
			}

			if (z > 0)
			{
				Block* bottBlock = blockIter.GetBottomNeighbor().GetBlock();
				uint8_t bottType = bottBlock->GetTypeIndex();

				uint8_t indoorVal = bottBlock->GetInDoorLightInfluence();
				uint8_t outdoorVal = bottBlock->GetOutDoorLightInfluence();
				redChannelVal = (unsigned char)RangeMapClamped((float)outdoorVal, 0.f, 15.f, 50.f, 255.f);
				greenChannelVal = (unsigned char)RangeMapClamped((float)indoorVal, 0.f, 15.f, 50.f, 255.f);

				bool isOpaque = BlockDef::GetDefByIndex(bottType)->m_isOpaque;
				if (!isOpaque)
				{
					AABB2 bottUV = spriteSheet->GetSpriteUVs(bottIndex);
					AddVertsForQuad3D(m_cpuMesh, blockBLB, blockBLF, blockBRF, blockBRB, Rgba8(redChannelVal, greenChannelVal, blueChannelVal), bottUV);
				}
			}

			//west & east face
			AABB2 sideUV = spriteSheet->GetSpriteUVs(sideIndex);

			Block* westBlock = blockIter.GetWestNeighbor().GetBlock();
			uint8_t westType = westBlock->GetTypeIndex();

			uint8_t indoorVal = westBlock->GetInDoorLightInfluence();
			uint8_t outdoorVal = westBlock->GetOutDoorLightInfluence();
			redChannelVal = (unsigned char)RangeMapClamped((float)outdoorVal, 0.f, 15.f, 50.f, 255.f);
			greenChannelVal = (unsigned char)RangeMapClamped((float)indoorVal, 0.f, 15.f, 50.f, 255.f);

			bool isOpaque = BlockDef::GetDefByIndex(westType)->m_isOpaque;
			if (!isOpaque)
			{
				AddVertsForQuad3D(m_cpuMesh, blockBLF, blockBLB, blockTLB, blockTLF, Rgba8(redChannelVal, greenChannelVal, blueChannelVal), sideUV);
			}

			Block* eastBlock = blockIter.GetEastNeighbor().GetBlock();
			uint8_t eastType = eastBlock->GetTypeIndex();

			indoorVal = eastBlock->GetInDoorLightInfluence();
			outdoorVal = eastBlock->GetOutDoorLightInfluence();
			redChannelVal = (unsigned char)RangeMapClamped((float)outdoorVal, 0.f, 15.f, 50.f, 255.f);
			greenChannelVal = (unsigned char)RangeMapClamped((float)indoorVal, 0.f, 15.f, 50.f, 255.f);

			isOpaque = BlockDef::GetDefByIndex(eastType)->m_isOpaque;
			if (!isOpaque)
			{
				AddVertsForQuad3D(m_cpuMesh, blockBRB, blockBRF, blockTRF, blockTRB, Rgba8(redChannelVal, greenChannelVal, blueChannelVal), sideUV);
			}

			//north & south

			
			Block* southBlock = blockIter.GetSouthNeighbor().GetBlock();
			uint8_t southType = southBlock->GetTypeIndex();
			isOpaque = BlockDef::GetDefByIndex(southType)->m_isOpaque;

			indoorVal = southBlock->GetInDoorLightInfluence();
			outdoorVal = southBlock->GetOutDoorLightInfluence();
			redChannelVal = (unsigned char)RangeMapClamped((float)outdoorVal, 0.f, 15.f, 50.f, 255.f);
			greenChannelVal = (unsigned char)RangeMapClamped((float)indoorVal, 0.f, 15.f, 50.f, 255.f);

			if (!isOpaque)
			{
				AddVertsForQuad3D(m_cpuMesh, blockBLB, blockBRB, blockTRB, blockTLB, Rgba8(redChannelVal, greenChannelVal, blueChannelVal), sideUV);
			}
			

			
			Block* northBlock = blockIter.GetNorthNeighbor().GetBlock();
			uint8_t northType = northBlock->GetTypeIndex();

			indoorVal = northBlock->GetInDoorLightInfluence();
			outdoorVal = northBlock->GetOutDoorLightInfluence();
			redChannelVal = (unsigned char)RangeMapClamped((float)outdoorVal, 0.f, 15.f, 50.f, 255.f);
			greenChannelVal = (unsigned char)RangeMapClamped((float)indoorVal, 0.f, 15.f, 50.f, 255.f);

			isOpaque = BlockDef::GetDefByIndex(northType)->m_isOpaque;
			if (!isOpaque)
			{
				AddVertsForQuad3D(m_cpuMesh, blockBRF, blockBLF, blockTLF, blockTRF, Rgba8(redChannelVal, greenChannelVal, blueChannelVal), sideUV);
			}
			
		}
		else
		{
			AABB2 topUV = spriteSheet->GetSpriteUVs(topIndex);
			AddVertsForQuad3D(m_cpuMesh, blockTLB, blockTRB, blockTRF, blockTLF, Rgba8::WHITE, topUV);
			AABB2 bottUV = spriteSheet->GetSpriteUVs(bottIndex);
			AddVertsForQuad3D(m_cpuMesh, blockBLB, blockBLF, blockBRF, blockBRB, Rgba8::WHITE, bottUV);
			AABB2 sideUV = spriteSheet->GetSpriteUVs(sideIndex);
			AddVertsForQuad3D(m_cpuMesh, blockBLF, blockBLB, blockTLB, blockTLF, Rgba8(230, 230, 230), sideUV);
			AddVertsForQuad3D(m_cpuMesh, blockBRB, blockBRF, blockTRF, blockTRB, Rgba8(230, 230, 230), sideUV);

			AddVertsForQuad3D(m_cpuMesh, blockBLB, blockBRB, blockTRB, blockTLB, Rgba8(200, 200, 200), sideUV);
			AddVertsForQuad3D(m_cpuMesh, blockBRF, blockBLF, blockTLF, blockTRF, Rgba8(200, 200, 200), sideUV);
		}
	}

	m_gpuMesh = g_theRenderer->CreateVertexBuffer(m_cpuMesh.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	g_theRenderer->CopyCPUToGPU(m_cpuMesh.data(), (size_t)m_cpuMesh.size() * m_gpuMesh->GetStride(), m_gpuMesh);

	m_isVertexDirty = false;
}


bool Chunk::ItHasToBeActivated(Vec2 PosXY)
{
	float dist;
	Vec2 concers[] =
	{
		Vec2(m_bounds.m_maxs.x, m_bounds.m_maxs.y),
		Vec2(m_bounds.m_maxs.x, m_bounds.m_mins.y),
		Vec2(m_bounds.m_mins.x, m_bounds.m_maxs.y),
		Vec2(m_bounds.m_mins.x, m_bounds.m_mins.y),
	};
	for (int concersIndex = 0; concersIndex < 4; concersIndex++)
	{
		dist = GetDistance2D(PosXY, concers[concersIndex]);
		if (dist < m_theWorld->m_chunkActivationRange)
		{
			return true;
		}
	}
	
	return false;
}

float Chunk::GetDistFromPos(Vec2 PosXY)
{
	AABB2 box = AABB2(Vec2(m_bounds.m_mins), Vec2(m_bounds.m_maxs));
	Vec2 center = box.GetCenter();
	return GetDistance2D(PosXY, center);
}

bool Chunk::ItHasToBeDeactivated(Vec2 PosXY)
{
	float deactivationRadius = m_theWorld->m_chunkActivationRange + CHUNK_BLOCK_NUM_OF_X + CHUNK_BLOCK_NUM_OF_Y;
	float dist = GetDistFromPos(PosXY);
	if (dist > deactivationRadius)
	{
		return true;
	}

	return false;
}

void Chunk::MarkDirty()
{
	m_isVertexDirty = true;
}

void Chunk::GenerateChunk()
{
	std::string folder = m_theWorld->worldFolder;
	std::string path = folder.append("/Chunk(");
	path.append(std::to_string(m_chunkCoords.x));
	path.append(",");
	path.append(std::to_string(m_chunkCoords.y));
	path.append(").chunk");
	std::ifstream inputFile(path, std::ios::binary);

	if (inputFile.good())
	{
		std::vector<unsigned char> fileData(std::istreambuf_iterator<char>(inputFile), {});
		inputFile.close();

		constexpr int MAX_VERTS = CHUNK_BLOCK_TOTAL * 6 * 2 * 3;
		m_cpuMesh.reserve(MAX_VERTS);
		unsigned int worldSeed = 0;
		worldSeed |= fileData[8];
		worldSeed <<= 8;
		worldSeed |= fileData[9];
		worldSeed <<= 8;
		worldSeed |= fileData[10];
		worldSeed <<= 8;
		worldSeed |= fileData[11];

		if (fileData[0] != 'G' || fileData[1] != 'C' || fileData[2] != 'H' || fileData[3] != 'K' || fileData[4] != m_theWorld->m_theGame->m_app->m_setting.version
			|| fileData[5] != CHUNK_SIZE_OF_X || fileData[6] != CHUNK_SIZE_OF_Y || fileData[7] != CHUNK_SIZE_OF_Z || worldSeed != m_theWorld->GetWorldSeed())
		{
			m_chunkState = ChunkState::ACTIVATING_QUEUED_GENERATE;
			ChunkGenerationJob* job = new ChunkGenerationJob(this);
			g_theJobSystem->QueueJob(job);
		}
		else
		{
			uint8_t type = 0;
			int blockIndex = 0;
			for (int bufferIndex = 12; bufferIndex < fileData.size(); ++bufferIndex)
			{
				if (bufferIndex % 2 == 0)
				{
					type = fileData[bufferIndex];
				}
				if (bufferIndex % 2 != 0)
				{
					for (int index = 0; index < fileData[bufferIndex]; ++index)
					{
						m_blocks[blockIndex] = type;
						blockIndex++;
					}
				}
			}
			m_isVertexDirty = true;
			m_theWorld->m_activeChunks.insert(std::make_pair(GetCoords(), this));
			m_theWorld->HookUpNeighbors(this);
			MarkNeighborDirtyIfTouchingNonOpaqueBoundary();
			DescendEachColumnFlaging_SKY();
			HorizontalSpread();
			MarkAllEmitsLightBlockDirty();
			std::vector<Chunk*>& list = m_theWorld->m_generatingChunks;
			for (int i = 0; i < list.size(); i++)
			{
				if (list[i] == this)
				{
					list[i] = nullptr;
				}
			}
			m_chunkState = ChunkState::ACTIVE;
		}


	}
	else
	{
		inputFile.close();
		m_chunkState = ChunkState::ACTIVATING_QUEUED_GENERATE;
		ChunkGenerationJob* job = new ChunkGenerationJob(this);
		g_theJobSystem->QueueJob(job);
		//InitializedBlocks();
	}
}

void Chunk::PutBlockTypeOnTop(Vec2 PosXY, uint8_t type)
{
	int coordx = (int)PosXY.x;
	int coordy = (int)PosXY.y;
	for (int blockIndex = 0; blockIndex < CHUNK_BLOCK_TOTAL; ++blockIndex)
	{
		int x = blockIndex & (CHUNK_BLOCK_NUM_OF_X - 1);
		int y = (blockIndex >> CHUNK_SIZE_OF_X) & (CHUNK_BLOCK_NUM_OF_X - 1);
		int z = blockIndex >> (CHUNK_SIZE_OF_X + CHUNK_SIZE_OF_Y);


		if (x != coordx || y != coordy)
		{
			continue;
		}
		else
		{
			if (z < CHUNK_BLOCK_NUM_OF_Z - 1)
			{
				int topBlockIndex = (z + 1) * (CHUNK_BLOCK_NUM_OF_X * CHUNK_BLOCK_NUM_OF_Y) + y * CHUNK_BLOCK_NUM_OF_X + x;
				uint8_t topType = m_blocks[topBlockIndex].GetTypeIndex();
				bool isOpaque = BlockDef::GetDefByIndex(topType)->m_isOpaque;
				if (!isOpaque)
				{
					m_blocks[topBlockIndex].ChangeTypeTo(type);
					MarkDirty();
					m_needsSaving = true;
					return;
				}
			}
		}
	}
}
int Chunk::GetIndexForWorldCoords(IntVec3 const& coords) const
{
	IntVec3 const& localCoords = GetLocalCoordsForWorldCoords(coords);
	return GetIndexForLocalCoords(localCoords);
}

IntVec3 Chunk::GetWorldCoordsForIndex(int index) const
{
	IntVec3 localCoords = GetLocalCoordsForIndex(index);
	int x = (m_chunkCoords.x << CHUNK_SIZE_OF_X) + localCoords.x;
	int y = (m_chunkCoords.y << CHUNK_SIZE_OF_Y) + localCoords.y;
	int z = localCoords.z;
	return IntVec3(x, y, z);
}

IntVec3 Chunk::GetLocalCoordsForIndex(int index) const
{
	int x = index & CHUNK_MASK_X;
	int y = index >> CHUNK_BITSHIFT_Y & CHUNK_MASK_Y;
	int z = index >> CHUNK_BITSHIFT_Z & CHUNK_MASK_Z;
	return IntVec3(x, y, z);
}

IntVec3 Chunk::GetLocalCoordsForWorldCoords(IntVec3 const& coords) const
{
	int x = coords.x & CHUNK_MASK_X;
	int y = coords.y & CHUNK_MASK_Y;
	return IntVec3(x, y, coords.z);
}

int Chunk::GetIndexForLocalCoords(IntVec3 const& coords) const
{
	return coords.x | coords.y << CHUNK_BITSHIFT_Y | coords.z << CHUNK_BITSHIFT_Z;
}

void Chunk::DigBlockTypeOnTop(Vec2 PosXY, uint8_t type)
{
	int coordx = (int)PosXY.x;
	int coordy = (int)PosXY.y;
	for (int blockIndex = 0; blockIndex < CHUNK_BLOCK_TOTAL; ++blockIndex)
	{
		int x = blockIndex & (CHUNK_BLOCK_NUM_OF_X - 1);
		int y = (blockIndex >> CHUNK_SIZE_OF_X) & (CHUNK_BLOCK_NUM_OF_X - 1);
		int z = blockIndex >> (CHUNK_SIZE_OF_X + CHUNK_SIZE_OF_Y);


		if (x != coordx || y != coordy)
		{
			continue;
		}
		else
		{
			if (z < CHUNK_BLOCK_NUM_OF_Z - 1)
			{
				int topBlockIndex = (z + 1) * (CHUNK_BLOCK_NUM_OF_X * CHUNK_BLOCK_NUM_OF_Y) + y * CHUNK_BLOCK_NUM_OF_X + x;
				uint8_t topType = m_blocks[topBlockIndex].GetTypeIndex();
				bool isOpaque = BlockDef::GetDefByIndex(topType)->m_isOpaque;
				if (!isOpaque)
				{
					m_blocks[blockIndex].ChangeTypeTo(type);
					MarkDirty();
					m_needsSaving = true;
					return;
				}
			}
		}
	}
}

size_t Chunk::GetSizeOfCPUMesh() const
{
	return m_cpuMesh.size();
}

Block* Chunk::GetBlockByIndex(int index) const
{
	return &m_blocks[index];
}

void Chunk::MarkNeighborDirtyIfTouchingNonOpaqueBoundary()
{
	if (m_eastNeighbor)
	{
		for (int blockIndex = 0; blockIndex < CHUNK_BLOCK_TOTAL; ++blockIndex)
		{
			int x = blockIndex & (CHUNK_BLOCK_NUM_OF_X - 1);

			if (x == 15)
			{
				if (!m_blocks[blockIndex].IsOpaque())
				{
					m_blocks[blockIndex].SetIsBlockLightDirtyTrue();
					m_theWorld->m_lightQueue.emplace_back(BlockIterator(this, blockIndex));
				}
			}
			else
			{
				continue;
			}
		}
	}

	if (m_southNeighbor)
	{
		for (int blockIndex = 0; blockIndex < CHUNK_BLOCK_TOTAL; ++blockIndex)
		{
			int x = blockIndex & (CHUNK_BLOCK_NUM_OF_X - 1);

			if (x == 0)
			{
				if (!m_blocks[blockIndex].IsOpaque())
				{
					m_blocks[blockIndex].SetIsBlockLightDirtyTrue();
					m_theWorld->m_lightQueue.emplace_back(BlockIterator(this, blockIndex));
				}
			}
			else
			{
				continue;
			}
		}
	}

	if (m_northNeighbor)
	{
		for (int blockIndex = 0; blockIndex < CHUNK_BLOCK_TOTAL; ++blockIndex)
		{
			int y = (blockIndex >> CHUNK_SIZE_OF_X) & (CHUNK_BLOCK_NUM_OF_X - 1);

			if (y == 15)
			{
				if (!m_blocks[blockIndex].IsOpaque())
				{
					m_blocks[blockIndex].SetIsBlockLightDirtyTrue();
					m_theWorld->m_lightQueue.emplace_back(BlockIterator(this, blockIndex));
				}
			}
			else
			{
				continue;
			}
		}
	}

	if (m_southNeighbor)
	{
		for (int blockIndex = 0; blockIndex < CHUNK_BLOCK_TOTAL; ++blockIndex)
		{
			int y = (blockIndex >> CHUNK_SIZE_OF_X) & (CHUNK_BLOCK_NUM_OF_X - 1);

			if (y == 0)
			{
				if (!m_blocks[blockIndex].IsOpaque())
				{
					m_blocks[blockIndex].SetIsBlockLightDirtyTrue();
					m_theWorld->m_lightQueue.emplace_back(BlockIterator(this, blockIndex));
				}
			}
			else
			{
				continue;
			}
		}
	}
}

void Chunk::DescendEachColumnFlaging_SKY()
{
	for (int x = 0; x < CHUNK_BLOCK_NUM_OF_X; ++x)
	{
		for (int y = 0; y < CHUNK_BLOCK_NUM_OF_Y; ++y)
		{
			for (int z = CHUNK_BLOCK_NUM_OF_Z - 1; z > 0; --z)
			{
				if (z == 0)
				{
					break;
				}
				int bottIndex = (z - 1) * (CHUNK_BLOCK_NUM_OF_X * CHUNK_BLOCK_NUM_OF_Y) + y * CHUNK_BLOCK_NUM_OF_X + x;
				if (m_blocks[bottIndex].IsOpaque())
				{
					for (int colIndex = CHUNK_BLOCK_NUM_OF_Z - 1; colIndex != z - 1; --colIndex)
					{
						int currentIndex = colIndex * (CHUNK_BLOCK_NUM_OF_X * CHUNK_BLOCK_NUM_OF_Y) + y * CHUNK_BLOCK_NUM_OF_X + x;
						m_blocks[currentIndex].SetIsBlockSkyTrue();

					}
				}
				else
				{
					continue;
				}
				break;
			}
		}
	}
}

void Chunk::HorizontalSpread()
{
	for (int x = 0; x < CHUNK_BLOCK_NUM_OF_X; ++x)
	{
		for (int y = 0; y < CHUNK_BLOCK_NUM_OF_Y; ++y)
		{
			for (int z = CHUNK_BLOCK_NUM_OF_Z - 1; z > 0; --z)
			{
				if (z == 0)
				{
					break;
				}
				int bottIndex = (z - 1) * (CHUNK_BLOCK_NUM_OF_X * CHUNK_BLOCK_NUM_OF_Y) + y * CHUNK_BLOCK_NUM_OF_X + x;
				if (m_blocks[bottIndex].IsOpaque())
				{
					for (int colIndex = CHUNK_BLOCK_NUM_OF_Z - 1; colIndex != z - 1; --colIndex)
					{
						int currentIndex = colIndex * (CHUNK_BLOCK_NUM_OF_X * CHUNK_BLOCK_NUM_OF_Y) + y * CHUNK_BLOCK_NUM_OF_X + x;
						BlockIterator blockIter = BlockIterator(this, currentIndex);
						Block* block = blockIter.GetBlock();
						if (block->IsBlockSky())
						{
							block->SetOutDoorLightInfluence(15);
						}
						else
						{
							continue;
						}

						BlockIterator const& east = blockIter.GetEastNeighbor();
						BlockIterator const& west = blockIter.GetWestNeighbor();
						BlockIterator const& north = blockIter.GetNorthNeighbor();
						BlockIterator const& south = blockIter.GetSouthNeighbor();

						Block* eastBlock = east.GetBlock();
						Block* westBlock = west.GetBlock();
						Block* northBlock = north.GetBlock();
						Block* southBlock = south.GetBlock();

						if (eastBlock)
						{
							if (!eastBlock->IsOpaque() && !eastBlock->IsBlockSky())
							{
								eastBlock->SetIsBlockLightDirtyTrue();
								m_theWorld->m_lightQueue.emplace_back(BlockIterator(east.m_owner, east.m_blockIndex));
							}
						}
						if (westBlock)
						{
							if (!westBlock->IsOpaque() && !westBlock->IsBlockSky())
							{
								westBlock->SetIsBlockLightDirtyTrue();
								m_theWorld->m_lightQueue.emplace_back(BlockIterator(west.m_owner, west.m_blockIndex));
							}
						}
						if (northBlock)
						{
							if (!northBlock->IsOpaque() && !northBlock->IsBlockSky())
							{
								northBlock->SetIsBlockLightDirtyTrue();
								m_theWorld->m_lightQueue.emplace_back(BlockIterator(north.m_owner, north.m_blockIndex));
							}
						}
						if (southBlock)
						{
							if (!southBlock->IsOpaque() && !southBlock->IsBlockSky())
							{
								southBlock->SetIsBlockLightDirtyTrue();
								m_theWorld->m_lightQueue.emplace_back(BlockIterator(south.m_owner, south.m_blockIndex));
							}
						}

					}
				}
				else
				{
					continue;
				}
				break;
			}
		}
	}
}

void Chunk::MarkAllEmitsLightBlockDirty()
{
	for (int blockIndex = 0; blockIndex < CHUNK_BLOCK_TOTAL; ++blockIndex)
	{
		if (m_blocks[blockIndex].IsEmitsLight())
		{
			m_theWorld->MarkLightingDirty(this, blockIndex);
		}
	}

}

IntVec2 Chunk::GetCoords() const
{
	return m_chunkCoords;
}

IntVec2 Chunk::GetNorth() const
{
	return IntVec2(m_chunkCoords.x, m_chunkCoords.y + 1);
}

IntVec2 Chunk::GetSouth() const
{
	return IntVec2(m_chunkCoords.x, m_chunkCoords.y - 1);
}

IntVec2 Chunk::GetEast() const
{
	return IntVec2(m_chunkCoords.x + 1, m_chunkCoords.y);
}

IntVec2 Chunk::GetWest() const
{
	return IntVec2(m_chunkCoords.x - 1, m_chunkCoords.y);
}

void Chunk::SaveData()
{
	std::string folder = m_theWorld->worldFolder;
	std::string path = folder.append("/Chunk(");
	path.append(std::to_string(m_chunkCoords.x));
	path.append(",");
	path.append(std::to_string(m_chunkCoords.y));
	path.append(").chunk");
	std::ofstream outputFile(path, std::ios::binary, std::ios::trunc);

	std::vector<unsigned char> buffer;
	buffer.push_back('G');
	buffer.push_back('C');
	buffer.push_back('H');
	buffer.push_back('K');
	buffer.push_back((unsigned char)m_theWorld->m_theGame->m_app->m_setting.version);
	buffer.push_back(CHUNK_SIZE_OF_X);
	buffer.push_back(CHUNK_SIZE_OF_Y);
	buffer.push_back(CHUNK_SIZE_OF_Z);
	unsigned int worldSeed = m_theWorld->GetWorldSeed();
	unsigned char pos1, pos2, pos3, pos4;
	pos1 = (worldSeed >> 24) & 0xFF;
	pos2 = (worldSeed >> 16) & 0xFF;
	pos3 = (worldSeed >> 8) & 0xFF;
	pos4 = worldSeed & 0xFF;
	buffer.push_back((pos1));
	buffer.push_back((pos2));
	buffer.push_back((pos3));
	buffer.push_back((pos4));

	char type = -1;
	unsigned char cnt = 1;
	for (int blockIndex = 0; blockIndex < CHUNK_BLOCK_TOTAL; blockIndex++)
	{
		if (m_blocks[blockIndex].GetTypeIndex() != type)
		{
			if (type == -1)
			{

			}
			else
			{
				buffer.push_back(cnt);
				cnt = 1;
			}
			type = m_blocks[blockIndex].GetTypeIndex();
			buffer.push_back(type);
		}
		else if (cnt < 255)
		{
			cnt++;
		}
		else if (cnt == 255)
		{
			buffer.push_back(cnt);
			buffer.push_back(type);
			cnt = 1;
		}

	}
	outputFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
	outputFile.close();
}

void Chunk::GenerateTreeAt(IntVec2 currentCoords)
{
	int worldX = CHUNK_BLOCK_NUM_OF_X * m_chunkCoords.x + currentCoords.x;
	int worldY = CHUNK_BLOCK_NUM_OF_Y * m_chunkCoords.y + currentCoords.y;
	int worldSeed = m_theWorld->m_theGame->m_app->m_setting.WorldSeed;
	float terrainNoise = Compute2dPerlinNoise(float(worldX), float(worldY), 200.f, 5, 0.5f, 2.0f, worldSeed);
	float humidity = 0.5f + 0.5f * Compute2dPerlinNoise(float(worldX), float(worldY), 800.f, 3, 2.75f, 2.0f, true, worldSeed + 1);
	float temperature = 0.5f + 0.5f * Compute2dPerlinNoise(float(worldX), float(worldY), 200.f, 3, 0.1f, 30.0f, true, worldSeed + 2);
	float hilliness = SmoothStep3(0.5f + 0.5f * Compute2dPerlinNoise(float(worldX), float(worldY), 1000.f, 9, 2.0f, 0.5f, true, worldSeed + 3));
	terrainNoise *= hilliness;
	float oceanness = SmoothStart3(Compute2dPerlinNoise(float(worldX), float(worldX), 1200.f, 2, 0.5f, 0.5f, true, worldSeed + 4));
	float terrainRawHeight = static_cast<float>((SEA_LEVEL - RIVER_WIDTH) + 50.f * fabsf((float)terrainNoise));
	int terrainHeight = static_cast<int>(RangeMapClamped(oceanness, 0.f, 0.5f, terrainRawHeight, MAX_DEPTH_SEA));

	if (terrainHeight == CHUNK_BLOCK_NUM_OF_Z)
	{
		return;
	}

	if (terrainHeight > SEA_LEVEL)
	{
		BlockTemplate* currentTemplate = BlockTemplate::GetTemplateByName("tree");
		if (humidity < HUMIDITY_THRESHOLD)
		{
			currentTemplate = BlockTemplate::GetTemplateByName("cactus");
		}
 		else if (temperature < TEMP_THRESHOLD)
 		{
 			currentTemplate = BlockTemplate::GetTemplateByName("spruce");
 		}
		IntVec3 root = IntVec3(currentCoords.x, currentCoords.y, terrainHeight + 1);
		

		for (int entryIndex = 0; entryIndex < currentTemplate->m_blockEntries.size(); ++entryIndex)
		{
			uint8_t thisType = currentTemplate->m_blockEntries[entryIndex].m_blockType;
			IntVec3 thisOffset = currentTemplate->m_blockEntries[entryIndex].m_offset;
			IntVec3 thisCroods = root + thisOffset;
			if (thisCroods.x < 0 || thisCroods.x > CHUNK_BLOCK_NUM_OF_X - 1 
				|| thisCroods.y < 0 || thisCroods.y > CHUNK_BLOCK_NUM_OF_Y - 1
				|| thisCroods.z < 0 || thisCroods.z > CHUNK_BLOCK_NUM_OF_Z - 1)
			{
				continue;
			}
			if (thisOffset == IntVec3(0, 0, 0))
			{
				int rootindex = root.x + root.y * CHUNK_BLOCK_NUM_OF_X + root.z * CHUNK_BLOCK_NUM_OF_Y * CHUNK_BLOCK_NUM_OF_X;
				BlockIterator iter = BlockIterator(this, rootindex);
				if (iter.GetBottomNeighbor().GetBlock())
				{
					if (iter.GetBottomNeighbor().GetBlock()->GetTypeIndex() == 0)
					{
						if (humidity < HUMIDITY_THRESHOLD)
						{
							iter.GetBottomNeighbor().GetBlock()->ChangeTypeTo(12);
						}
						else if (temperature < TEMP_THRESHOLD)
						{
							iter.GetBottomNeighbor().GetBlock()->ChangeTypeTo(20);
						}
						else
						{
							iter.GetBottomNeighbor().GetBlock()->ChangeTypeTo(3);
						}
					}
				}
			}
			int index = thisCroods.x + thisCroods.y * CHUNK_BLOCK_NUM_OF_X + thisCroods.z * CHUNK_BLOCK_NUM_OF_Y * CHUNK_BLOCK_NUM_OF_X;
			m_blocks[index].ChangeTypeTo(thisType);
		}
	}
	else if (terrainHeight == SEA_LEVEL)
	{

	}
}



void Chunk::GenerateHouseAt()
{
	int worldSeed = m_theWorld->m_theGame->m_app->m_setting.WorldSeed;

	RandomNumberGenerator localRNG = RandomNumberGenerator();
	localRNG.m_seed = m_theWorld->GetWorldSeed() + m_chunkCoords.x * (m_chunkCoords.y * 10);
	int randomX = localRNG.RollRandomIntInRange(5, 11);
	int randomY = localRNG.RollRandomIntInRange(5, 11);
	int chunkGlobalX = m_chunkCoords.x << CHUNK_SIZE_OF_X;
	int chunkGlobalY = m_chunkCoords.y << CHUNK_SIZE_OF_Y;
	float globalX = static_cast<float>(chunkGlobalX + randomX);
	float globalY = static_cast<float>(chunkGlobalY + randomY);

	float terrainNoise = Compute2dPerlinNoise(float(globalX), float(globalY), 200.f, 5, 0.5f, 2.0f, worldSeed);
	float humidity = 0.5f + 0.5f * Compute2dPerlinNoise(float(globalX), float(globalY), 800.f, 3, 2.75f, 2.0f, true, worldSeed + 1);
	float temperature = 0.5f + 0.5f * Compute2dPerlinNoise(float(globalX), float(globalY), 200.f, 3, 0.1f, 30.0f, true, worldSeed + 2);
	float hilliness = SmoothStep3(0.5f + 0.5f * Compute2dPerlinNoise(float(globalX), float(globalY), 1000.f, 9, 2.0f, 0.5f, true, worldSeed + 3));
	terrainNoise *= hilliness;
	float oceanness = SmoothStart3(Compute2dPerlinNoise(float(globalX), float(globalY), 1200.f, 2, 0.5f, 0.5f, true, worldSeed + 4));
	float terrainRawHeight = static_cast<float>((SEA_LEVEL - RIVER_WIDTH) + 50.f * fabsf((float)terrainNoise));
	int terrainHeight = static_cast<int>(RangeMapClamped(oceanness, 0.f, 0.5f, terrainRawHeight, MAX_DEPTH_SEA));

	if (terrainHeight == CHUNK_BLOCK_NUM_OF_Z)
	{
		return;
	}

	if (terrainHeight > SEA_LEVEL)
	{
		BlockTemplate const* templateDef = BlockTemplate::GetTemplateByName("house");
		IntVec3 root = IntVec3(randomX, randomY, terrainHeight + 1);
		for (int templateBlockIndex = 0; templateBlockIndex < (int)templateDef->m_blockEntries.size(); templateBlockIndex++)
		{
			BlockTemplateEntry const& templateBlock = templateDef->m_blockEntries[templateBlockIndex];
			uint8_t thisType = templateDef->m_blockEntries[templateBlockIndex].m_blockType;
			IntVec3 blockCoords = root + templateBlock.m_offset;
			if (blockCoords.x < 0 || blockCoords.x > CHUNK_MASK_X || blockCoords.y < 0 || blockCoords.y > CHUNK_MASK_Y) continue;
			int blockIndex = GetIndexForLocalCoords(blockCoords);
			m_blocks[blockIndex].ChangeTypeTo(thisType);

			if (blockCoords.z == terrainHeight + 1)
			{
				BlockIterator iter = BlockIterator(this, blockIndex);
				bool isTop = true;
				if (iter.GetBottomNeighbor().GetBlock())
				{
					while (iter.GetBottomNeighbor().GetBlock()->GetTypeIndex() == 0)
					{
						if (humidity < HUMIDITY_THRESHOLD)
						{
							iter.GetBottomNeighbor().GetBlock()->ChangeTypeTo(12);
						}
						else if (temperature < TEMP_THRESHOLD)
						{
							iter.GetBottomNeighbor().GetBlock()->ChangeTypeTo(20);
						}
						else
						{
							if (isTop)
							{
								iter.GetBottomNeighbor().GetBlock()->ChangeTypeTo(3);
								isTop = false;
							}
							else
							{
								iter.GetBottomNeighbor().GetBlock()->ChangeTypeTo(2);
							}
							
						}
						iter = iter.GetBottomNeighbor();
					}
				}
			}

		}
	}
}

bool Chunk::IsLocalCoordsTreenessLocalMax(IntVec2 currentCoords, std::map<IntVec2, float> const& treenessMap)
{
	int currentX = currentCoords.x;
	int currentY = currentCoords.y;

	auto Iter = treenessMap.find(currentCoords);
	float currentTreeNoise = Iter->second;

	for (int neighborY = currentY - 2; neighborY < currentY + 2; neighborY++)
	{
		for (int neighborX = currentX - 2; neighborX < currentX + 2; neighborX++)
		{
			IntVec2 Coords(neighborX, neighborY);
			if (Coords == currentCoords)
			{
				continue;
			}
			auto neighbor = treenessMap.find(Coords);
			if (neighbor == treenessMap.end()) continue;

			float searchTreeNoise = neighbor->second;

			if (searchTreeNoise >= currentTreeNoise)
			{
				return false;
			}
		}
	}
	return true;
}

void Chunk::AddCaves(RandomNumberGenerator& rng, unsigned int seed)
{
	UNUSED(rng);

	constexpr float CHANCE_FOR_CAVE_TO_START_IN_A_CHUNK = 0.05f;
	constexpr float CAVE_MAX_DISTENCE_BLOCKS = 300.f;
	
	constexpr float CHUNK_WIDTH = CHUNK_BLOCK_NUM_OF_X < CHUNK_BLOCK_NUM_OF_Y ? static_cast<float>(CHUNK_BLOCK_NUM_OF_X) : static_cast<float>(CHUNK_BLOCK_NUM_OF_Y);
	constexpr int CAVE_SEARCH_RADIUS = 1 + int(CAVE_MAX_DISTENCE_BLOCKS / CHUNK_WIDTH);

	IntVec2 chunkSearchMins = m_chunkCoords - IntVec2(CAVE_SEARCH_RADIUS, CAVE_SEARCH_RADIUS);
	IntVec2 chunkSearchMaxs = m_chunkCoords + IntVec2(CAVE_SEARCH_RADIUS, CAVE_SEARCH_RADIUS);
	for (int chunkY = chunkSearchMins.y; chunkY <= chunkSearchMaxs.y; ++chunkY)
	{
		for (int chunkX = chunkSearchMins.x; chunkX <= chunkSearchMaxs.x; ++chunkX)
		{
			float caveOriginationNoise = Get2dNoiseZeroToOne(chunkX, chunkY, seed);
			if (caveOriginationNoise < CHANCE_FOR_CAVE_TO_START_IN_A_CHUNK)
			{
				//m_chunkCroodsWhereCaveStart.emplace_back(chunkX, chunkY);
				m_nearbyCaves.push_back(CaveInfo(IntVec2(chunkX, chunkY)));
			}
		}
	}

	for (int i = 0; i < m_nearbyCaves.size(); i++)
	{
		CaveInfo& cave = m_nearbyCaves[i];
		unsigned int seedForThisCave = Get2dNoiseUint(cave.m_startChunkCoords.x, cave.m_startChunkCoords.y, seed);
		RandomNumberGenerator caveRNG;
		caveRNG.m_seed = seedForThisCave;

		EulerAngles crawlOrientation;

		crawlOrientation.m_yawDegrees = caveRNG.RollRandomFloatInRange(0.f, 360.f);
		crawlOrientation.m_pitchDegrees = caveRNG.RollRandomFloatInRange(-30.f, 30.f);
		crawlOrientation.m_rollDegrees = 0.f;

		AABB3 startChunkBounds = GetChunkBoundsForChunkCoords(cave.m_startChunkCoords);
		cave.m_startGlobalPos.x = caveRNG.RollRandomFloatInRange(startChunkBounds.m_mins.x, startChunkBounds.m_maxs.x);
		cave.m_startGlobalPos.y = caveRNG.RollRandomFloatInRange(startChunkBounds.m_mins.y, startChunkBounds.m_maxs.y);
		cave.m_startGlobalPos.z = caveRNG.RollRandomFloatInRange(40.f, 60.f);
		Vec3 crawlWordPos;
		crawlWordPos = cave.m_startGlobalPos;

		for (int crawStep = 0; crawStep < 5; ++crawStep)
		{
			Vec3 forwardDisp = CAVE_STEP_LENGTH * crawlOrientation.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
			crawlWordPos += forwardDisp;
			crawlOrientation.m_yawDegrees += caveRNG.RollRandomFloatInRange(-CAVE_MAX_TURN_DEGREES, CAVE_MAX_TURN_DEGREES);
			crawlOrientation.m_pitchDegrees += Compute1dPerlinNoise(float(crawStep), 10.f, 3, 0.5f, 2.f, true, seed + 1);

			cave.m_caveNodePositions.push_back(crawlWordPos);
		}
	}

	for (int i = 0; i < m_nearbyCaves.size(); i++)
	{
		CaveInfo& cave = m_nearbyCaves[i];
		for (int nodeIndex = 0; nodeIndex < cave.m_caveNodePositions.size() - 1; ++nodeIndex)
		{

			int nextIndex = nodeIndex + 1;
			Vec3 start = cave.m_caveNodePositions[nodeIndex];
			Vec3 end = cave.m_caveNodePositions[nextIndex];
			Vec3 nearPosStart = m_bounds.GetNearestPoint(start);
			Vec3 nearPosEnd = m_bounds.GetNearestPoint(end);
			if (IsPointInsideCapsule3D(nearPosStart, start, end, CAVE_CAPSULE_RADIUS) || IsPointInsideCapsule3D(nearPosEnd, start, end, CAVE_CAPSULE_RADIUS))
			{
				CaveCapsule capsule;
				capsule.start = start;
				capsule.end = end;
				m_caves.push_back(capsule);
			}
		}

	}

}

void Chunk::DebugAddVertsForCaves(std::vector<Vertex_PCU>& verts) const
{
	UNUSED(verts);
	Vec3 markerPos = m_bounds.GetCenter() + Vec3(0.f, 0.f, 80.f);
}

bool Chunk::IsCoordsVillageLocalMax(std::map<IntVec2, float> const& houseNoise) const
{
	std::map<IntVec2, float>::const_iterator curItr = houseNoise.find(m_chunkCoords);
	float currentTreeNoise = curItr->second;
	for (int chunkY = -HOUSE_RADIUS; chunkY < HOUSE_RADIUS; chunkY++)
	{
		for (int chunkX = -HOUSE_RADIUS; chunkX < HOUSE_RADIUS; chunkX++)
		{
			IntVec2 searchCoords = m_chunkCoords + IntVec2(chunkX, chunkY);
			if (searchCoords == m_chunkCoords) continue;
			std::map<IntVec2, float>::const_iterator searchItr = houseNoise.find(searchCoords);
			float searchTreeNoise = searchItr->second;
			if (searchTreeNoise >= currentTreeNoise)
			{
				return false;
			}
		}
	}
	return true;
}

AABB3 Chunk::GetChunkBoundsForChunkCoords(IntVec2 m_startChunkCoords)
{
	AABB3 bounds;
	bounds.m_mins.x = (float)CHUNK_BLOCK_NUM_OF_X * (float)m_startChunkCoords.x;
	bounds.m_mins.y = (float)CHUNK_BLOCK_NUM_OF_Y * (float)m_startChunkCoords.y;
	bounds.m_mins.z = 0.f;

	bounds.m_maxs.x = (float)(CHUNK_BLOCK_NUM_OF_X) * (float)m_startChunkCoords.x + (float)CHUNK_BLOCK_NUM_OF_X;
	bounds.m_maxs.y = (float)(CHUNK_BLOCK_NUM_OF_Y) * (float)m_startChunkCoords.y + (float)CHUNK_BLOCK_NUM_OF_Y;
	bounds.m_maxs.z = (float)CHUNK_BLOCK_NUM_OF_Z;

	return bounds;
}

