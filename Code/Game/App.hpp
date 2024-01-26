#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

constexpr int MAX_NUM_KEYCODE = 256;
bool static m_isQuitting = false;

struct gameSetting
{
	bool autoCreateChunks = false;
	float chunkActivationDistance = 10.f;
	bool saveModifiedChunks = false;
	bool hiddenSurfaceRemoval = false;
	int version = 1;
	Rgba8 globalIndoorLightColor;
	Rgba8 globalOutdoorLightColor;
	Rgba8 skyColor;
	float fogFarDistance = 10.f;
	float fogNearDistance = 0.f;
	float raycastMaxDist = 0.f;
	unsigned int WorldSeed = 0;
};

class App
{
	friend class Game;
public :
	App();
	~App();
	void Startup();
	void Run();
	void Shutdown();
	void Runframe();
		
	bool IsQuitting() const { return m_isQuitting;  }
	bool HandleQuitRequested();
	bool OpenSlowMo();
	bool CloseSlowMo();
	bool SwitchPaused();
	void MoveOneStepThenPaused();
	bool const IsKeyDown(unsigned char keyCode);
	bool const WasKeyJustPressed(unsigned char keyCode);
	void ShowAttractMode();
	bool GetIsAttractMo();
	SoundPlaybackID GetBGMPlaybackID() const;
	Camera* m_screenCamera = nullptr;
    gameSetting m_setting;
	
private:
	void BeginFrame();
	void Update(float deltaSeconds);
	void Render() const;
	void EndFrame();

	void UpdateShip(float deltaSeconds);
	void CopyIsDownToWasDown();
	void LoadingGameConfig();

private:
	Game* m_theGame;
	SoundPlaybackID BGM;
	
};