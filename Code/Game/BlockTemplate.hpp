#pragma once
#include <stdint.h>
#include "Engine/Math/IntVec3.hpp"
#include <vector>
#include <string>

constexpr int TREE_RADIUS = 3;
constexpr int HOUSE_RADIUS = 5;

struct BlockTemplateEntry
{
public:
	BlockTemplateEntry(uint8_t type, IntVec3 offset);

	uint8_t m_blockType = 0;
	IntVec3 m_offset;
};


class BlockTemplate
{
public:
	BlockTemplate();
	~BlockTemplate();

	static void InitializeBlockTemplates();
	static BlockTemplate* GetTemplateByName(std::string const& name);
	static std::vector<BlockTemplate*> s_blockTemplates;

	

public:
	std::string m_name;
	std::vector<BlockTemplateEntry> m_blockEntries;
};