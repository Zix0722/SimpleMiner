#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include <vector>

class App;
class Clock;
class Player;
class Entity;
class Stopwatch;
class Prop;
class World;
class Chunk;


struct simpleMinerConstants
{
	Vec4 CamWorldPosition;
	Vec4 GlobalIndoorLightColor;
	Vec4 GlobalOutdoorLightColor;
	Vec4 SkyColor;
	float FogFarDistance;
	float FogNearDistance;
	float DayTime;
	float padding;
 };

class Game
{
public:
	Game(App* owner);
	~Game();
	void Startup();
	void Update(float deltaSeconds);
	void Render() const;
	void EndFrame();
	void Shutdown();
public :
	bool m_isAttractMode = true;
	bool g_DebugMo = false;
	App* m_app = nullptr;
	Player* m_player = nullptr;
	World* m_currentWorld = nullptr;
	Shader* m_shader = nullptr;
	bool m_isTestMode = false;
	void ShowAttractMode() const;
	void LoadAssets();
	void RandomizeDate();
	Chunk* GetChunkByPlayerPos(Vec3 pos) const;
	void DigTheHighestOnPos(Vec2 pos);
	void PutTheHighestOnPos(Vec2 pos, uint8_t type);
	void UpdateGameBuffer(Vec3 camPos);
	void UpdateOutDoorLight(Rgba8 outDoorLight);
	void UpdateInDoorLight(Rgba8 inDoorLight);
	void UpdateSkyLight(Rgba8 skyLight);
	//------------test job system purpose---------------------
	void TestingJobSystemFunction();
	void TestingModeUpdate();
	void TestingModeRender() const;
	void TestStart();
	//--------------------------------------------------------
private:
	bool m_isTestStart = false;
	std::vector<AABB2*> m_tiles;
	void UpdateWorldCamera(float deltaSeconds);
	void UpdateScreenCamera(float deltaSeconds);
	void RenderAxes() const;
	void InitialGameBuffer();
	void ChangingLightAndSkyColorByTime();
private:
	float m_backAttractCounter = 0.f;
	Clock* m_gameClock = nullptr;
	std::vector<Entity*> m_entities;
	simpleMinerConstants m_gameConstantBuffer;
	ConstantBuffer* m_gameBuffer = nullptr;
	Rgba8 m_skyColor;
	Rgba8 m_outdoorColor;
	Rgba8 m_indoorColor;
};