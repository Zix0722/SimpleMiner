#include "Block.hpp"
#include "BlockDef.hpp"

Block::Block()
{

}

Block::Block(uint8_t type)
	:m_typeIndex(type)
{

}


Block::~Block()
{

}

uint8_t Block::GetTypeIndex() const
{
	return m_typeIndex;
}

void Block::ChangeTypeTo(uint8_t type)
{
	m_typeIndex = type;
}

bool Block::IsOpaque() const
{
	BlockDef* def = BlockDef::GetDefByIndex(m_typeIndex);
	return def->m_isOpaque;
}

bool Block::IsSolid() const
{
	BlockDef* def = BlockDef::GetDefByIndex(m_typeIndex);
	return def->m_isSolid;
}

bool Block::IsVisible() const
{
	BlockDef* def = BlockDef::GetDefByIndex(m_typeIndex);
	return def->m_isVisible;
}

bool Block::IsEmitsLight() const
{
	BlockDef* def = BlockDef::GetDefByIndex(m_typeIndex);
	if (def->m_indoorLight != 0 || def->m_outdoorLight != 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// void Block::SetOutDoorLightInfluence(int val)
// {
// 	m_lightInfluenceData = (m_lightInfluenceData & 0x0F) | (val << 4);
// }
// 
// void Block::SetInDoorLightInfluence(int val)
// {
// 	m_lightInfluenceData = (m_lightInfluenceData & 0xF0) | (val & 0x0F);
// }
// 
// uint8_t Block::GetOutDoorLightInfluence()
// {
// 	return (m_lightInfluenceData >> 4) & 0x0F;
// }
// 
// uint8_t Block::GetInDoorLightInfluence()
// {
// 	return m_lightInfluenceData & 0x0F;
// }

bool Block::IsBlockSky() const
{
	return (m_bitflags & 0x80) != 0;
}

void Block::SetIsBlockSkyTrue()
{
	m_bitflags |= 0x80;
}

void Block::SetIsBlockSkyFalse()
{
	m_bitflags &= 0x7F;
}


bool Block::IsBlockLightDirty() const
{
	return (m_bitflags & 0x40) != 0;
}


void Block::SetIsBlockLightDirtyTrue()
{
	m_bitflags |= 0x40;
}

void Block::SetIsBlockLightDirtyFalse()
{
	m_bitflags &= 0xBF;
}

bool Block::IsBlockFullOpaque() const
{
	return (m_bitflags & 0x20) != 0;
}

void Block::SetIsBlockFullOpaque(bool isFullOpaque)
{
	if (isFullOpaque)
	{
		m_bitflags |= 0x20;
	}
	else
	{
		m_bitflags &= 0xDF;
	}
}

bool Block::IsBlockSolid() const
{
	return (m_bitflags & 0x10) != 0;
}

void Block::SetIsBlockSolid(bool isSolid)
{
	if (isSolid)
	{
		m_bitflags |= 0x10;
	}
	else
	{
		m_bitflags &= 0xEF;
	}
}

bool Block::IsBlockVisible() const
{
	return (m_bitflags & 0x08) != 0;
}

void Block::SetIsBlockVisible(bool isVisible)
{
	if (isVisible)
	{
		m_bitflags |= 0x08;
	}
	else
	{
		m_bitflags &= 0xF7;
	}
}

