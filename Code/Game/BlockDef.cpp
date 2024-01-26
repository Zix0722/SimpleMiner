#include "BlockDef.hpp"

std::vector<BlockDef*> BlockDef::s_blockDefinitions;

void BlockDef::CreateNewBlockDef(std::string const& name, bool visible, bool solid, bool opaque, IntVec2 const& topSprite, IntVec2 const& sidesSprite, IntVec2 const& bottSprite, uint8_t outdoorLight, uint8_t indoorLight)
{
	BlockDef* newBlockDef = new BlockDef();
	newBlockDef->m_name = name;
	newBlockDef->m_isVisible = visible;
	newBlockDef->m_isSolid = solid;
	newBlockDef->m_isOpaque = opaque;
	newBlockDef->m_UVCoords_Top = topSprite;
	newBlockDef->m_UVCoords_Sides = sidesSprite;
	newBlockDef->m_UVCoords_Bottom = bottSprite;
	newBlockDef->m_outdoorLight = outdoorLight;
	newBlockDef->m_indoorLight = indoorLight;

	s_blockDefinitions.push_back(newBlockDef);
}

BlockDef::BlockDef()
{
	
}

BlockDef::~BlockDef()
{

}

void BlockDef::InitializeBlockDefs()
{
	//----------Data-Driven Later-----------------------

	//                name  visible solid  opaque   topSprite     sidesSprite    bottSprite 
	CreateNewBlockDef("air", false, false, false, IntVec2(0, 0), IntVec2(0, 0), IntVec2(0, 0));
	CreateNewBlockDef("stone", true, true, true, IntVec2(33, 32), IntVec2(33, 32), IntVec2(33, 32));
	CreateNewBlockDef("dirt", true, true, true, IntVec2(32, 34), IntVec2(32, 34), IntVec2(32, 34));
	CreateNewBlockDef("grass", true, true, true, IntVec2(32, 33), IntVec2(33, 33), IntVec2(32, 34));
	CreateNewBlockDef("coal", true, true, true, IntVec2(63, 34), IntVec2(63, 34), IntVec2(63, 34));
	CreateNewBlockDef("iron", true, true, true, IntVec2(63, 35), IntVec2(63, 35), IntVec2(63, 35));
	CreateNewBlockDef("gold", true, true, true, IntVec2(63, 36), IntVec2(63, 36), IntVec2(63, 36));
	CreateNewBlockDef("diamond", true, true, true, IntVec2(63, 37), IntVec2(63, 37), IntVec2(63, 37));
	CreateNewBlockDef("water", true, false, true, IntVec2(32, 44), IntVec2(32, 44), IntVec2(32, 44));
	CreateNewBlockDef("bricks", true, true, true, IntVec2(57, 32), IntVec2(57, 32), IntVec2(57, 32));
	CreateNewBlockDef("cobblestone", true, true, true, IntVec2(35, 32), IntVec2(35, 32), IntVec2(35, 32));
	CreateNewBlockDef("glowstone", true, true, true, IntVec2(46,34), IntVec2(46, 34), IntVec2(46, 34), 0, 15);
	CreateNewBlockDef("sand", true, true, true, IntVec2(34, 34), IntVec2(34, 34), IntVec2(34, 34));
	CreateNewBlockDef("ice", true, true, true, IntVec2(45, 34), IntVec2(45, 34), IntVec2(45, 34));
	CreateNewBlockDef("log", true, true, true, IntVec2(38, 33), IntVec2(36, 33), IntVec2(38, 33));
	CreateNewBlockDef("leaves", true, true, true, IntVec2(32, 35), IntVec2(32, 35), IntVec2(32, 35));
	CreateNewBlockDef("cactus", true, true, true, IntVec2(38, 36), IntVec2(37, 36), IntVec2(38, 36));
	CreateNewBlockDef("cactusTop", true, true, true, IntVec2(39, 36), IntVec2(37, 36), IntVec2(38, 36));
	CreateNewBlockDef("coldLog", true, true, true, IntVec2(38, 33), IntVec2(37, 33), IntVec2(38, 33));
	CreateNewBlockDef("coldLeaves", true, true, true, IntVec2(34, 35), IntVec2(34, 35), IntVec2(34, 35));
	CreateNewBlockDef("snow", true, true, true, IntVec2(36, 35), IntVec2(33, 35), IntVec2(32, 34));
	CreateNewBlockDef("woodWall", true, true, true, IntVec2(39, 33), IntVec2(39, 33), IntVec2(39, 33));
}

BlockDef* BlockDef::GetDefByIndex(uint8_t index)
{
	return s_blockDefinitions[index];
}

