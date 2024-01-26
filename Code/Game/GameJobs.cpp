#include "GameJobs.hpp"
#include "Game/Chunk.hpp"
#include "Game/Game.hpp"

ChunkGenerationJob::ChunkGenerationJob(Chunk* chunk)
	:m_chunk(chunk)
{

}

void ChunkGenerationJob::Execute()
{
	m_chunk->InitializedBlocks();
}

TestJob::TestJob(Game* game, int index)
	:m_game(game)
	,m_index(index)
{

}

void TestJob::Execute()
{
	m_game->TestingJobSystemFunction();
}
