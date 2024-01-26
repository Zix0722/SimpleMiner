#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Clock.hpp"
#include "Player.hpp"
#include "Prop.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "World.hpp"
#include "BlockDef.hpp"
#include "Game/Chunk.hpp"
#include <cmath>
#include "BlockTemplate.hpp"
#include "GameJobs.hpp"


RandomNumberGenerator* g_theRNG = nullptr;
extern Renderer* g_theRenderer;
extern DevConsole* g_theDevConsole;

static const int k_gameConstantsSlot = 4;

Game::Game(App* owner)
	:m_app(owner)
{
	g_theRNG = new RandomNumberGenerator();
	m_gameClock = new Clock();
}

Game::~Game()
{
	delete m_gameClock;
	delete m_currentWorld;
}

void Game::Startup()
{
	LoadAssets();

	m_player = new Player(this);
	m_entities.push_back(m_player);
	m_player->m_position.z = 90.f;
	m_currentWorld = new World(this);
	m_currentWorld->Startup();
	UpdateWorldCamera(0.f);
	InitialGameBuffer();
}

void Game::Update(float deltaSeconds)
{
	if (m_isAttractMode)
	{
		UNUSED(deltaSeconds);
		return;
	}

	if (m_isTestMode)
	{
		TestingModeUpdate();
		return;
	}

	ChangingLightAndSkyColorByTime();
	

	UpdateSkyLight(m_skyColor);
	UpdateOutDoorLight(m_outdoorColor);
	UpdateInDoorLight(m_indoorColor);


	for (int entityIndex = 0; entityIndex < m_entities.size(); ++entityIndex)
	{
		if (m_entities[entityIndex] != nullptr)
		{
			m_entities[entityIndex]->Update(deltaSeconds);
		}
	}
	
	m_currentWorld->Update(deltaSeconds);
	UpdateGameBuffer(m_player->m_camera->m_position);
}

void Game::Render() const
{
	if (m_isAttractMode)
	{
		g_theRenderer->BeginCamera(*m_app->m_screenCamera);
		ShowAttractMode();
		g_theDevConsole->Render(AABB2(m_app->m_screenCamera->GetOrthoBottomLeft(), m_app->m_screenCamera->GetOrthoTopRight()));
		g_theRenderer->EndCamera(*m_app->m_screenCamera);
		return;
	}
	else
	{
		if (m_isTestMode)
		{
			TestingModeRender();
			return;
		}

		g_theRenderer->ClearScreen(m_skyColor);
		g_theRenderer->BeginCamera(*m_player->m_camera);


		g_theRenderer->CopyCPUToGPU(&m_gameConstantBuffer, sizeof(simpleMinerConstants), m_gameBuffer);
		g_theRenderer->BindConstantBuffer(k_gameConstantsSlot, m_gameBuffer);

		for (int entityIndex = 0; entityIndex < m_entities.size(); ++entityIndex)
		{
			if (m_entities[entityIndex] != nullptr)
			{
				m_entities[entityIndex]->Render();
			}
		}
		g_theRenderer->BindShader(m_shader);
		m_currentWorld->Render();
		RenderAxes();
		g_theRenderer->EndCamera(*m_player->m_camera);

		DebugRenderWorld(*m_player->m_camera);
		DebugRenderScreen(*m_app->m_screenCamera);

		g_theRenderer->BeginCamera(*m_app->m_screenCamera);
		g_theDevConsole->Render(AABB2(m_app->m_screenCamera->GetOrthoBottomLeft(), m_app->m_screenCamera->GetOrthoTopRight()));
		g_theRenderer->EndCamera(*m_app->m_screenCamera);
		return;
	}
}


void Game::EndFrame()
{
	
}

void Game::Shutdown()
{

}


void Game::UpdateWorldCamera(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	m_player->m_camera->SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 75.f, 0.1f, 1000.0f);
	m_player->m_camera->SetRenderBasis(Vec3(0.f,0.f,1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f));
}

void Game::UpdateScreenCamera(float deltaSeconds)
{
	UNUSED (deltaSeconds);
	m_app->m_screenCamera->SetOrthoView(Vec2(0.f, 0.f), Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y));
}



void Game::RenderAxes() const
{
	std::vector<Vertex_PCU> verts;
	// world axes x y z
	AddVertsForLineSegment3D(verts, Vec3(0.f, 0.f, 0.f), Vec3(1.0f, 0.f, 0.f), 0.05f, Rgba8::RED);
	AddVertsForLineSegment3D(verts, Vec3(0.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f), 0.05f, Rgba8::GREEN);
	AddVertsForLineSegment3D(verts, Vec3(0.f, 0.f, 0.f), Vec3(0.f, 0.f, 1.f), 0.05f, Rgba8::BLUE);
	// camera axes x y z 
	if (g_DebugMo)
	{
		Vec3 xline = Vec3::MakeFromPolarDegrees(0.f, 0.f, 0.2f);
		Vec3 yline = Vec3::MakeFromPolarDegrees(0.f, 90.f, 0.2f);
		Vec3 zline = Vec3::MakeFromPolarDegrees(90.f, 0.f, 0.2f);

		Mat44 playerMat = m_player->m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
		Vec3 iForward = playerMat.GetIBasis3D();
		Vec3 startPoint = m_player->m_position + iForward * 0.2f;
		AddVertsForLineSegment3D(verts, startPoint, startPoint + Vec3(0.01f, 0.f, 0.f), 0.001f, Rgba8::RED);
		AddVertsForLineSegment3D(verts, startPoint, startPoint + Vec3(0.f, 0.01f, 0.f), 0.001f, Rgba8::GREEN);
		AddVertsForLineSegment3D(verts, startPoint, startPoint + Vec3(0.f, 0.f, 0.01f), 0.001f, Rgba8::BLUE);
	}

	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
	g_theRenderer->EndCamera(*m_player->m_camera);
}

void Game::InitialGameBuffer()
{
	m_gameBuffer = g_theRenderer->CreateConstantBuffer(sizeof(simpleMinerConstants));

	m_gameConstantBuffer.FogFarDistance = m_app->m_setting.fogFarDistance;
	m_gameConstantBuffer.FogNearDistance = m_app->m_setting.fogNearDistance;

	float colorFloat[4];
	m_app->m_setting.skyColor.GetAsFloats(colorFloat);
 	m_gameConstantBuffer.SkyColor = Vec4(colorFloat[0], colorFloat[1], colorFloat[2], colorFloat[3]);

	m_app->m_setting.globalIndoorLightColor.GetAsFloats(colorFloat);
	m_gameConstantBuffer.GlobalIndoorLightColor = Vec4(colorFloat[0], colorFloat[1], colorFloat[2], colorFloat[3]);

	m_app->m_setting.globalOutdoorLightColor.GetAsFloats(colorFloat);
	m_gameConstantBuffer.GlobalOutdoorLightColor = Vec4(colorFloat[0], colorFloat[1], colorFloat[2], colorFloat[3]);
}

void Game::ChangingLightAndSkyColorByTime()
{
	float dayfraction = fmodf(m_currentWorld->m_worldTime, 1.f);
	if (dayfraction < 0.25f || dayfraction > 0.75f)
	{
		m_skyColor = Rgba8(20, 20, 40);
	}
	else if (dayfraction < 0.5f)
	{
		unsigned char r = (unsigned char)RangeMapClamped(dayfraction, 0.25f, 0.5f, 20.f, 200.f);
		unsigned char g = (unsigned char)RangeMapClamped(dayfraction, 0.25f, 0.5f, 20.f, 230.f);
		unsigned char b = (unsigned char)RangeMapClamped(dayfraction, 0.25f, 0.5f, 40.f, 255.f);
		m_skyColor = Rgba8(r, g, b);
	}
	else
	{
		unsigned char r = (unsigned char)RangeMapClamped(dayfraction, 0.5f, 0.75f, 200.f, 20.f);
		unsigned char g = (unsigned char)RangeMapClamped(dayfraction, 0.5f, 0.75f, 230.f, 20.f);
		unsigned char b = (unsigned char)RangeMapClamped(dayfraction, 0.5f, 0.75f, 255.f, 40.f);
		m_skyColor = Rgba8(r, g, b);
	}

	if (dayfraction < 0.25f || dayfraction > 0.75f)
	{
		m_outdoorColor = Rgba8(20, 20, 40);
	}
	else if (dayfraction < 0.5f)
	{
		unsigned char r = (unsigned char)RangeMapClamped(dayfraction, 0.25f, 0.5f, 20.f, 255.f);
		unsigned char g = (unsigned char)RangeMapClamped(dayfraction, 0.25f, 0.5f, 20.f, 255.f);
		unsigned char b = (unsigned char)RangeMapClamped(dayfraction, 0.25f, 0.5f, 40.f, 255.f);
		m_outdoorColor = Rgba8(r, g, b);
	}
	else
	{
		unsigned char r = (unsigned char)RangeMapClamped(dayfraction, 0.5f, 0.75f, 255.f, 20.f);
		unsigned char g = (unsigned char)RangeMapClamped(dayfraction, 0.5f, 0.75f, 255.f, 20.f);
		unsigned char b = (unsigned char)RangeMapClamped(dayfraction, 0.5f, 0.75f, 255.f, 40.f);
		m_outdoorColor = Rgba8(r, g, b);
	}

	float lightingStrength = m_currentWorld->m_lightingStrength;
	unsigned char outdoorR = (unsigned char)RangeMapClamped(lightingStrength, 0.f, 1.f, (float)m_outdoorColor.r, 255.f);
	unsigned char outdoorG = (unsigned char)RangeMapClamped(lightingStrength, 0.f, 1.f, (float)m_outdoorColor.g, 255.f);
	unsigned char outdoorB = (unsigned char)RangeMapClamped(lightingStrength, 0.f, 1.f, (float)m_outdoorColor.b, 255.f);
	m_outdoorColor = Rgba8(outdoorR, outdoorG, outdoorB);

	unsigned char SkyR = (unsigned char)RangeMapClamped(lightingStrength, 0.f, 1.f, (float)m_skyColor.r, 255.f);
	unsigned char SkyG = (unsigned char)RangeMapClamped(lightingStrength, 0.f, 1.f, (float)m_skyColor.g, 255.f);
	unsigned char SkyB = (unsigned char)RangeMapClamped(lightingStrength, 0.f, 1.f, (float)m_skyColor.b, 255.f);
	m_skyColor = Rgba8(SkyR, SkyG, SkyB);

	float indoorStrength = m_currentWorld->m_glowStrength;

	m_indoorColor.r = (unsigned char)((float)m_app->m_setting.globalIndoorLightColor.r * indoorStrength);
	m_indoorColor.g = (unsigned char)((float)m_app->m_setting.globalIndoorLightColor.g * indoorStrength);
	m_indoorColor.b = (unsigned char)((float)m_app->m_setting.globalIndoorLightColor.b * indoorStrength);
	m_indoorColor.a = (unsigned char)((float)m_app->m_setting.globalIndoorLightColor.a * indoorStrength);

}

void Game::ShowAttractMode() const
{
	g_theRenderer->ClearScreen(Rgba8(100, 100, 100));
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->BindTexture(&g_theFont->GetTexture());
	g_theRenderer->SetModelConstants();
	std::vector<Vertex_PCU> titileVerts;
	AABB2 titleBox = AABB2(Vec2(0.f,0.f), Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y));
	g_theFont->AddVertsForTextInBox2D(titileVerts, titleBox, 70.f, "Simple Miner", Rgba8::GREEN, 1.f, Vec2(0.5f, 0.7f));

	float t = Clock::GetSystemClock().GetTotalSeconds();
	float tAfterSin = sinf(3.f * t);
	unsigned char a = (unsigned char)RangeMapClamped(tAfterSin, 0.f, 1.f, 0.f, 255.f);

	g_theFont->AddVertsForTextInBox2D(titileVerts, titleBox, 20.f, "Press Space To Start", Rgba8(255,255,255,a), 1.f, Vec2(0.5f, 0.3f));
	g_theRenderer->DrawVertexArray((int)titileVerts.size(), titileVerts.data());
}

void Game::LoadAssets()
{
	m_shader = g_theRenderer->CreateShader("Data/Shader/World");
	BlockDef::InitializeBlockDefs();
	BlockTemplate::InitializeBlockTemplates();
	g_theRenderer->CreateOrGetTextureFromFile("Data/Image/BasicSprites_64x64.png");
}

void Game::RandomizeDate() 
{
	m_currentWorld->DeleteAllChunks();
	m_app->m_setting.WorldSeed++;
	if (!m_currentWorld->CheckIfWorldFolderExist())
	{
		m_currentWorld->ForceCreateWorldFolder();
	}
}

Chunk* Game::GetChunkByPlayerPos(Vec3 pos) const
{
	Vec2 playerPos2D = Vec2(pos);
	float xBefore = (playerPos2D.x / (CHUNK_BLOCK_NUM_OF_X * 1.f));
	float yBefore = (playerPos2D.y / (CHUNK_BLOCK_NUM_OF_Y * 1.f));
	int chunkX, chunkY;
	if (xBefore >= 0.f)
	{
		chunkX = static_cast<int>(std::floor(xBefore));
	}
	else
	{
		chunkX = static_cast<int>(std::floor(xBefore));
	}
	if (yBefore >= 0.f)
	{
		chunkY = static_cast<int>(std::floor(yBefore));
	}
	else
	{
		chunkY = static_cast<int>(std::floor(yBefore));
	}

	IntVec2 chunkKey = IntVec2(chunkX, chunkY);
	if (m_currentWorld->m_activeChunks.count(chunkKey) > 0)
	{
		return m_currentWorld->m_activeChunks[chunkKey];
	}
	return nullptr;
}

void Game::DigTheHighestOnPos(Vec2 pos)
{
	Chunk* chunk = GetChunkByPlayerPos(m_player->m_position);
	float x = (float)chunk->GetCoords().x * (float)CHUNK_BLOCK_NUM_OF_X;
	float y = (float)chunk->GetCoords().y * (float)CHUNK_BLOCK_NUM_OF_Y;
	Vec2 localPos = pos - Vec2(x, y);

	chunk->DigBlockTypeOnTop(localPos, 0);
}

void Game::PutTheHighestOnPos(Vec2 pos, uint8_t type)
{
	Chunk* chunk = GetChunkByPlayerPos(m_player->m_position);
	float x = (float)chunk->GetCoords().x * (float)CHUNK_BLOCK_NUM_OF_X;
	float y = (float)chunk->GetCoords().y * (float)CHUNK_BLOCK_NUM_OF_Y;
	Vec2 localPos = pos - Vec2(x, y);
	chunk->PutBlockTypeOnTop(localPos, type);
}

void Game::UpdateGameBuffer(Vec3 camPos)
{
	m_gameConstantBuffer.CamWorldPosition.x = camPos.x;
	m_gameConstantBuffer.CamWorldPosition.y = camPos.y;
	m_gameConstantBuffer.CamWorldPosition.z = camPos.z;
	m_gameConstantBuffer.CamWorldPosition.w = 1.f;
}

void Game::UpdateOutDoorLight(Rgba8 outDoorLight)
{
	float colorFloat[4];
	outDoorLight.GetAsFloats(colorFloat);
	m_gameConstantBuffer.GlobalOutdoorLightColor = Vec4(colorFloat[0], colorFloat[1], colorFloat[2], colorFloat[3]);
}

void Game::UpdateInDoorLight(Rgba8 inDoorLight)
{
	float colorFloat[4];
	inDoorLight.GetAsFloats(colorFloat);
	m_gameConstantBuffer.GlobalIndoorLightColor = Vec4(colorFloat[0], colorFloat[1], colorFloat[2], colorFloat[3]);
}

void Game::UpdateSkyLight(Rgba8 skyLight)
{
	float colorFloat[4];
	skyLight.GetAsFloats(colorFloat);
	m_gameConstantBuffer.SkyColor = Vec4(colorFloat[0], colorFloat[1], colorFloat[2], colorFloat[3]);
}

void Game::TestingJobSystemFunction()
{
	float duration = g_theRNG->RollRandomFloatInRange(50.f, 3000.f);
	Sleep(static_cast<DWORD>(duration));
}

void Game::TestingModeUpdate()
{
	if (g_theInput->WasKeyJustPressed('1'))
	{
		g_theJobSystem->RetrieveCompletedJobs();
	}

	if (g_theInput->WasKeyJustPressed('2'))
	{
		Job* completedJob2 = g_theJobSystem->RetrieveCompletedJobs();
		while (completedJob2)
		{
			if (TestJob* job = dynamic_cast<TestJob*>(completedJob2))
			{
				
			}

			delete completedJob2;
			completedJob2 = g_theJobSystem->RetrieveCompletedJobs();
		}
	}
	if (g_theInput->WasKeyJustPressed('U'))
	{
		TestStart();
	}
}

void Game::TestingModeRender() const
{
	g_theRenderer->BeginCamera(*m_app->m_screenCamera);

	g_theRenderer->ClearScreen(Rgba8(100, 100, 100));
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	std::vector<Vertex_PCU> tilesVerts;
	tilesVerts.reserve(6 * 1000);
 	
	if (m_isTestStart)
	{
		for (int i = 0; i < g_theJobSystem->m_jobsToDoList.size(); i++)
		{
			TestJob* job = dynamic_cast<TestJob*>(g_theJobSystem->m_jobsToDoList[i]);
			if (!job)
			{
				continue;
			}
			AABB2* current = m_tiles[job->m_index];
			
			float x, y;
			x = static_cast<float>(job->m_index % 40);
			y = static_cast<float>(job->m_index / 40);
			current->SetCenter(Vec2(20.f, 20.f) + Vec2(x * 32.f, y * 32.f));
			AddVertsForAABB2(tilesVerts, *current, Rgba8::RED);
		}
		for (int i = 0; i < g_theJobSystem->m_retrievedJobs.size(); i++)
		{
			TestJob* job = dynamic_cast<TestJob*>(g_theJobSystem->m_retrievedJobs[i]);
			if (!job)
			{
				continue;
			}
			AABB2* current = m_tiles[job->m_index];
			float x, y;
			x = static_cast<float>(job->m_index % 40);
			y = static_cast<float>(job->m_index / 40);
			current->SetCenter(Vec2(20.f, 20.f) + Vec2(x * 32.f, y * 32.f));
			AddVertsForAABB2(tilesVerts, *current, Rgba8::YELLOW);
		}
		for (int i = 0; i < g_theJobSystem->m_completedJobs.size(); i++)
		{
			TestJob* job = dynamic_cast<TestJob*>(g_theJobSystem->m_completedJobs[i]);
			if (!job)
			{
				continue;
			}
			AABB2* current = m_tiles[job->m_index];
			float x, y;
			x = static_cast<float>(job->m_index % 40);
			y = static_cast<float>(job->m_index / 40);
			current->SetCenter(Vec2(20.f, 20.f) + Vec2(x * 32.f, y * 32.f));
			AddVertsForAABB2(tilesVerts, *current, Rgba8::GREEN);
 		}
	}
	

	g_theRenderer->DrawVertexArray((int)tilesVerts.size(), tilesVerts.data());
	g_theRenderer->EndCamera(*m_app->m_screenCamera);
}

void Game::TestStart()
{
	if (m_isTestStart)
	{
		return;
	}
  	for (int i = 0; i < 800; i++)
  	{
  		TestJob* job = new TestJob(this, i);
  		g_theJobSystem->QueueJob(job);
  	}
	

	m_tiles.reserve(1000);

	for (int i = 0; i < 800; i++)
	{
		m_tiles.push_back(new AABB2());
	}

	for (int i = 0; i < 800; i++)
	{
		AABB2*& current = m_tiles[i];
		current->SetDimensions(Vec2(30.f, 30.f));
	}
	Vec2 startPos = Vec2(20.f, 20.f);

	for (int a = 0; a < 20; a++)
	{
		for (int b = 0; b < 40; b++)
		{
			float x = startPos.x + 32.f * b;
			float y = startPos.y + 32.f * a;
			int index = a * b;
			AABB2*& current = m_tiles[index];
			current->SetCenter(Vec2(x, y));
		}
	}
	m_isTestStart = true;
}




