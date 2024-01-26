#pragma once
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/JobSystem.hpp"

struct Vec2;
extern InputSystem* g_theInput;
extern RandomNumberGenerator* g_theRNG;
extern AudioSystem* g_theAudio;
extern Renderer* g_theRenderer; 
extern Window* g_theWindow;
extern JobSystem* g_theJobSystem;
extern BitmapFont* g_theFont;

constexpr float WORLD_SIZE_X = 400.f;
constexpr float WORLD_SIZE_Y = 200.f;
constexpr float WORLD_CAMERA_SIZE_X = 200.f;
constexpr float WORLD_CAMERA_SIZE_Y = 100.f;
constexpr float SCREEN_CAMERA_SIZE_X = 1600.f;
constexpr float SCREEN_CAMERA_SIZE_Y = 800.f;


constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;
constexpr float SCRREN_CENTER_Y = SCREEN_CAMERA_SIZE_Y / 2.f;
constexpr float SCRREN_CENTER_X = SCREEN_CAMERA_SIZE_X / 2.f;

//Simple Miner--------------------------------------------------------
constexpr int CHUNK_SIZE_OF_X = 4;
constexpr int CHUNK_SIZE_OF_Y = 4;
constexpr int CHUNK_SIZE_OF_Z = 7;

constexpr int CHUNK_BLOCK_NUM_OF_X = 16;
constexpr int CHUNK_BLOCK_NUM_OF_Y = 16;
constexpr int CHUNK_BLOCK_NUM_OF_Z = 128;

constexpr int CHUNK_MAX_X = CHUNK_BLOCK_NUM_OF_X - 1;
constexpr int CHUNK_MAX_Y = CHUNK_BLOCK_NUM_OF_Y - 1;
constexpr int CHUNK_MAX_Z = CHUNK_BLOCK_NUM_OF_Z - 1;

constexpr int CHUNK_MASK_X = CHUNK_MAX_X;
constexpr int CHUNK_MASK_Y = CHUNK_MAX_Y;
constexpr int CHUNK_MASK_Z = CHUNK_MAX_Z;
constexpr int CHUNK_BITSHIFT_X = 0;
constexpr int CHUNK_BITSHIFT_Y = CHUNK_SIZE_OF_X;
constexpr int CHUNK_BITSHIFT_Z = CHUNK_SIZE_OF_Y + CHUNK_SIZE_OF_X;

constexpr int CHUNK_BLOCK_TOTAL = CHUNK_BLOCK_NUM_OF_X * CHUNK_BLOCK_NUM_OF_Y * CHUNK_BLOCK_NUM_OF_Z;


void	DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color);
void	DebugDrawLine(Vec2 const& startPos, Vec2 const& endPos, float thickness, Rgba8 const& color);

