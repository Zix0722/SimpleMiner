#include "BlockIterator.hpp"
#include "Game/Chunk.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


BlockIterator::BlockIterator()
{

}

BlockIterator::BlockIterator(Chunk* owner, int index)
	:m_blockIndex(index)
	,m_owner(owner)
{

}

BlockIterator::~BlockIterator()
{

}

Block* BlockIterator::GetBlock() const
{
	if (m_owner == nullptr || m_owner->m_blocks == nullptr || m_blockIndex < 0 || m_blockIndex > CHUNK_BLOCK_TOTAL)
	{
		return nullptr;
	}
	else
	{
		return m_owner->GetBlockByIndex(m_blockIndex);
	}
}

Vec3 BlockIterator::GetWorldCenter() const
{
	int x = m_blockIndex & (CHUNK_BLOCK_NUM_OF_X - 1);
	int y = (m_blockIndex >> CHUNK_SIZE_OF_X) & (CHUNK_BLOCK_NUM_OF_X - 1);
	int z = m_blockIndex >> (CHUNK_SIZE_OF_X + CHUNK_SIZE_OF_Y);

	IntVec2 currentChunkCoords = m_owner->GetCoords();

	Vec3 result;
	result.x = currentChunkCoords.x * CHUNK_BLOCK_NUM_OF_X + x + 0.5f;
	result.y = currentChunkCoords.y * CHUNK_BLOCK_NUM_OF_Y + y + 0.5f;
	result.z = z + 0.5f;

	return result;
}

BlockIterator BlockIterator::GetEastNeighbor() const
{
	if (m_owner == nullptr)
	{
		return  BlockIterator();
	}

	int x = m_blockIndex & (CHUNK_MAX_X);

	if (x == CHUNK_MAX_X)
	{
		BlockIterator result = BlockIterator(m_owner->m_eastNeighbor, m_blockIndex & ~CHUNK_MAX_X);
		return result;
	}
	else
	{
		BlockIterator result = BlockIterator(m_owner, m_blockIndex + 1);
		return result;
	}

}

BlockIterator BlockIterator::GetWestNeighbor() const
{
	if (m_owner && ((m_blockIndex & CHUNK_MAX_X) == 0))
	{
		return BlockIterator(m_owner->m_westNeighbor, m_blockIndex | CHUNK_MAX_X);
	}
	else
	{
		return BlockIterator(m_owner, m_blockIndex - 1);
	}
}

BlockIterator BlockIterator::GetNorthNeighbor() const
{
	if (m_owner && (((m_blockIndex >> CHUNK_BITSHIFT_Y) & CHUNK_MASK_Y) == CHUNK_MASK_Y))
	{
		return BlockIterator(m_owner->m_northNeighbor, m_blockIndex & ~(CHUNK_MASK_Y << CHUNK_BITSHIFT_Y));
	}
	else
	{
		return BlockIterator(m_owner, m_blockIndex + CHUNK_BLOCK_NUM_OF_X);
	}
}

BlockIterator BlockIterator::GetSouthNeighbor() const
{
	if (m_owner && (((m_blockIndex >> CHUNK_BITSHIFT_Y) & CHUNK_MASK_Y) == 0))
	{
		return BlockIterator(m_owner->m_southNeighbor, m_blockIndex | (CHUNK_MASK_Y << CHUNK_BITSHIFT_Y));
	}
	else
	{
		return BlockIterator(m_owner, m_blockIndex - CHUNK_BLOCK_NUM_OF_X);
	}

}

BlockIterator BlockIterator::GetTopNeighbor() const
{
	if (m_blockIndex + CHUNK_BLOCK_NUM_OF_X * CHUNK_BLOCK_NUM_OF_Y > CHUNK_BLOCK_TOTAL)
	{
		return BlockIterator();
	}
	return BlockIterator(m_owner, m_blockIndex + CHUNK_BLOCK_NUM_OF_X * CHUNK_BLOCK_NUM_OF_Y);
}

BlockIterator BlockIterator::GetBottomNeighbor() const
{
	if (m_blockIndex - CHUNK_BLOCK_NUM_OF_X * CHUNK_BLOCK_NUM_OF_Y < 0)
	{
		return BlockIterator();
	}
	return BlockIterator(m_owner, m_blockIndex - CHUNK_BLOCK_NUM_OF_X * CHUNK_BLOCK_NUM_OF_Y);
}

