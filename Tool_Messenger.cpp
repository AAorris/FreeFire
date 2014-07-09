#include "stdafx.h"
#include "Tool_Messenger.h"
#include "Tool_Configurable.h"
#include <sstream>
#include <boost\property_tree\ptree.hpp>
#include <boost\property_tree\info_parser.hpp>

#include <SDL2\SDL_net.h>

/*=======================================
-----------------------------------------
			IMPLEMENTATION
-----------------------------------------
========================================*/

Tool_Messenger::Impl::Impl()
{

}

std::string Tool_Messenger::Impl::read(Uint8* start)
{
	auto input = std::vector<char>();
	char* c = new char();
	char* c1 = new char();
	Uint16* val = new Uint16();
	int misses = 0;
	do {
		*val = SDLNet_Read16(start + input.size() + misses);
		c = (char*)val;
		c1 = (char*)val + 1;
		if (*c != '\0' && *c != -1)
			input.push_back(*c);
		else
			misses++;
		if (*c1 != '\0' && *c != -1)
			input.push_back(*c1);
		else
			misses++;
	} while ((*c != -1) && (input.size() < 512));
	return std::string(input.begin(),input.end());
}

void Tool_Messenger::Impl::write(const std::string& data, Uint8* start)
{
	int i = 0;
	for (i = 0; i+1 < data.size(); i+=2)
	{
		Uint16 val = 0;
		Uint8* ref = (Uint8*)&val;
		ref[0] = data[i];
		ref[1] = data[i+1];
		SDLNet_Write16(val, start + i );
	}
	SDLNet_Write16(0xFFFF, start + i++);
}


/*=======================================
-----------------------------------------
			   Definition
-----------------------------------------
========================================*/

Tool_Messenger::Tool_Messenger()
{

}

Tool_Messenger::~Tool_Messenger()
{

}

std::string Tool_Messenger::read(Uint8* start)
{
	return p->read(start);
}

void Tool_Messenger::write(const std::string& data, Uint8* start)
{
	p->write(data, start);
}