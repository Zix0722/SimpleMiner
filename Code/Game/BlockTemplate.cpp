#include "BlockTemplate.hpp"

std::vector<BlockTemplate*> BlockTemplate::s_blockTemplates;



BlockTemplateEntry::BlockTemplateEntry(uint8_t type, IntVec3 offset)
	:m_offset(offset)
	, m_blockType(type)
{

}


BlockTemplate::BlockTemplate()
{

}

BlockTemplate::~BlockTemplate()
{

}

void BlockTemplate::InitializeBlockTemplates()
{
	uint8_t log = 14;
	uint8_t leaves = 15;
	uint8_t cactus = 16;
	uint8_t cactusTop = 17;
	uint8_t clodtree = 18;
	uint8_t clodleaves = 19;
	uint8_t woodWall = 21;
	uint8_t glowStone = 11;
	uint8_t air = 0;
	uint8_t brick = 9;
	uint8_t cobblestone = 10;
	BlockTemplate* tree = new BlockTemplate();
	

	tree->m_name = "tree";
	s_blockTemplates.emplace_back(tree);
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(log, IntVec3(0, 0, 0)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(log, IntVec3(0, 0, 1)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(log, IntVec3(0, 0, 2)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(log, IntVec3(0, 0, 3)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(log, IntVec3(0, 0, 4)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(0, 0, 5)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(1, 0, 5)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(-1, 0, 5)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(0, -1, 5)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(0, 1, 5)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(0, -1, 4)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(0, 1, 4)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(1, 0, 4)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(-1, 0, 4)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(1, 1, 4)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(-1, -1, 4)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(1, -1, 4)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(-1, 1, 4)));

	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(0, -1, 3)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(0, 1, 3)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(1, 0, 3)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(-1, 0, 3)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(1, 1, 3)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(-1, -1, 3)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(1, -1, 3)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(-1, 1, 3)));
 
 	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(0, -2, 3)));
 	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(0, 2, 3)));
 	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(2, 0, 3)));
 	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(-2, 0, 3)));
 	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(2, 1, 3)));
 	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(-2, 1, 3)));
 	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(2, -1, 3)));
 	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(-2, -1, 3)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(1, 2, 3)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(-1, 2, 3)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(1, -2, 3)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(-1, -2, 3)));

	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(0, -2, 2)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(0, 2, 2)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(2, 0, 2)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(-2, 0, 2)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(2, 1, 2)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(-2, 1, 2)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(2, -1, 2)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(-2, -1, 2)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(1, 2, 2)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(-1, 2, 2)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(1, -2, 2)));
	tree->m_blockEntries.emplace_back(BlockTemplateEntry(leaves, IntVec3(-1, -2, 2)));
	//-----------cactus------------------------------------------------------------
	BlockTemplate* cactusTemp = new BlockTemplate();
	cactusTemp->m_name = "cactus";
	s_blockTemplates.emplace_back(cactusTemp);
	cactusTemp->m_blockEntries.emplace_back(BlockTemplateEntry(cactus, IntVec3(0, 0, 0)));
	cactusTemp->m_blockEntries.emplace_back(BlockTemplateEntry(cactus, IntVec3(0, 0, 1)));
	cactusTemp->m_blockEntries.emplace_back(BlockTemplateEntry(cactusTop, IntVec3(0, 0, 2)));

	BlockTemplate* spruceTemp = new BlockTemplate;
	spruceTemp->m_name = "spruce";
	s_blockTemplates.emplace_back(spruceTemp);

	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodtree, IntVec3(0, 0, 0)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodtree, IntVec3(0, 0, 1)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodtree, IntVec3(0, 0, 2)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodtree, IntVec3(0, 0, 3)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodtree, IntVec3(0, 0, 4)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(0, 0, 5)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(1, 0, 5)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(-1, 0, 5)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(0, -1, 5)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(0, 1, 5)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(0, -1, 4)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(0, 1, 4)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(1, 0, 4)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(-1, 0, 4)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(1, 1, 4)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(-1, -1, 4)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(1, -1, 4)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(-1, 1, 4)));

	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(0, -1, 3)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(0, 1, 3)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(1, 0, 3)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(-1, 0, 3)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(1, 1, 3)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(-1, -1, 3)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(1, -1, 3)));
	spruceTemp->m_blockEntries.emplace_back(BlockTemplateEntry(clodleaves, IntVec3(-1, 1, 3)));

	//------------------house---------------------------------------------------------------
	BlockTemplate* houseTemp = new BlockTemplate;
	houseTemp->m_name = "house";
	s_blockTemplates.emplace_back(houseTemp);
	std::vector<BlockTemplateEntry>& templist = houseTemp->m_blockEntries;

	constexpr int width = 3, length = 3, height = 5;
	for (int z = 0; z < height; ++z)
	{
		for (int y = -length; y < length; ++y)
		{
			for (int x = -width; x < width; ++x)
			{
				if (x == -width || x == width - 1 || y == length - 1 || y == -length || z == height -1)
				{
					if (z == height - 1)
					{
						templist.emplace_back(brick, IntVec3(x, y, z));
					}
					else if (z == height - 2)
					{
						if ((x == -width && (y == length - 1 ||  y == -length)) || (x == width - 1 && (y == length - 1 || y == -length)))
						{
							templist.emplace_back(brick, IntVec3(x, y, z));
						}
					}
					else

					{
						templist.emplace_back(woodWall, IntVec3(x, y, z));
					}
				}
				else
				{
					templist.emplace_back(air, IntVec3(x, y, z));
				}
			}
		}
	}

	templist.emplace_back(cobblestone, IntVec3(-1, 0, 0));
	templist.emplace_back(glowStone, IntVec3(-1, 0, 1));
	templist.emplace_back(cobblestone, IntVec3(0, 0, 0));
	templist.emplace_back(glowStone, IntVec3(0, 0, 1));
	templist.emplace_back(air, IntVec3(-1, -length, 0));
	templist.emplace_back(air, IntVec3(-1, -length, 1));
	templist.emplace_back(air, IntVec3(0, -length, 0));
	templist.emplace_back(air, IntVec3(0, -length, 1));
}

BlockTemplate* BlockTemplate::GetTemplateByName(std::string const& name)
{
	for (int templateIndex = 0; templateIndex < s_blockTemplates.size(); ++templateIndex)
	{
		BlockTemplate* temp = s_blockTemplates[templateIndex];
		if (temp->m_name == name)
		{
			return temp;
		}
	}

	return nullptr;
}
