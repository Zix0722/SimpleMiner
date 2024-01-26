#pragma once
#include "Engine/Math/Vec3.hpp"
class Block;
class Chunk;



struct BlockIterator

{
public:
	BlockIterator();
	BlockIterator(Chunk* owner, int index);
	~BlockIterator();

	Block* GetBlock() const;
	Vec3 GetWorldCenter() const;

	BlockIterator GetEastNeighbor() const;
	BlockIterator GetWestNeighbor() const;
	BlockIterator GetNorthNeighbor() const;
	BlockIterator GetSouthNeighbor() const;
	BlockIterator GetTopNeighbor() const;
	BlockIterator GetBottomNeighbor() const;

public:
	int m_blockIndex = -1;
	Chunk* m_owner = nullptr;
};