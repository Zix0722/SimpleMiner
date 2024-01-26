#pragma once
#include "Engine/Core/JobWorker.hpp"

class Chunk;
class Game;

class ChunkGenerationJob : public Job
{
public:
	ChunkGenerationJob(Chunk* chunk);
	~ChunkGenerationJob() {}

	virtual void Execute() override;
public:
	Chunk* m_chunk = nullptr;
};

class TestJob : public Job
{
public:
	TestJob(Game* game, int index);
	~TestJob() {}

	virtual void Execute() override;
public:
	Game* m_game = nullptr;
	int m_index = -1;
};