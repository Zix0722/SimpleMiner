#pragma once
#include <vector>
#include "Engine/Math/IntVec2.hpp"
#include <string>
class BlockDef
{
public:
	BlockDef();
	~BlockDef();

	static void InitializeBlockDefs();
	static BlockDef* GetDefByIndex(uint8_t index);
	static std::vector<BlockDef*> s_blockDefinitions;
public:
	std::string m_name;
	bool m_isVisible = false;
	bool m_isSolid = false;
	bool m_isOpaque = false;
	IntVec2 m_UVCoords_Top;
	IntVec2 m_UVCoords_Sides;
	IntVec2 m_UVCoords_Bottom;
	uint8_t m_outdoorLight = 0;
	uint8_t m_indoorLight = 0;
private:
	static void CreateNewBlockDef(std::string const& name, bool visible, bool solid, bool opaque, IntVec2 const& topSprite, IntVec2 const& sidesSprite, IntVec2 const& bottSprite, uint8_t outdoorLight = 0.f, uint8_t indoorLight = 0.f);
};