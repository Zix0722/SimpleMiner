#pragma once
#include <stdint.h>
class Block
{
public:
	Block();
	Block(uint8_t type);
	~Block();
	
	uint8_t GetTypeIndex() const;
	void ChangeTypeTo(uint8_t type);

	bool IsOpaque() const;
	bool IsSolid() const;
	bool IsVisible() const;
	bool IsEmitsLight() const;

	void SetOutDoorLightInfluence(uint8_t val) {m_lightInfluenceData = (m_lightInfluenceData & 0x0F) | (val << 4);};
	void SetInDoorLightInfluence(uint8_t val)  {m_lightInfluenceData = (m_lightInfluenceData & 0xF0) | (val & 0x0F);};

	uint8_t GetOutDoorLightInfluence() {return (m_lightInfluenceData >> 4) & 0x0F;};
	uint8_t GetInDoorLightInfluence()  {return m_lightInfluenceData & 0x0F;};

	bool IsBlockSky() const;
	void SetIsBlockSkyTrue();
	void SetIsBlockSkyFalse();

	bool IsBlockLightDirty() const;
	void SetIsBlockLightDirtyTrue();
	void SetIsBlockLightDirtyFalse();

	bool IsBlockFullOpaque() const;
	void SetIsBlockFullOpaque(bool isFullOpaque);

	bool IsBlockSolid() const;
	void SetIsBlockSolid(bool isSolid);

	bool IsBlockVisible() const;
	void SetIsBlockVisible(bool isVisible);

private:
	uint8_t m_typeIndex = 0;
	uint8_t m_lightInfluenceData = 0;
	uint8_t m_bitflags = 0;
};