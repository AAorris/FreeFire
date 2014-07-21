#include "stdafx.h"
#include "Tool_Data.h"
#include "scalar.h"


Tool_Data::Tool_Data() : id{ 0 }, flags{ 0 }
{
	pos = scalar(0, 0);
	burning = false;
}

Tool_Data::Tool_Data(char tileID, unsigned long startFlags, std::vector<std::string> assetData)
: id{ tileID }, flags{ startFlags }
{
	pos = scalar{};
	burning = false;
	for (auto item : assetData)
	{
		assets.insert(item);
	}
}

Tool_Data::~Tool_Data()
{
}
