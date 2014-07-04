#pragma once
#include "stdafx.h"
//#include "Role.h"
#include "Tool_Configurable.h"
#include "Tool_Asset.h"
#include "Tool_Messenger.h"
#include <SDL2\SDL_net.h>
#include <bitset>
#include <cassert>
//#include <sstream>


class Pos
{
	int		_pos;
public:
	short* x;
	short* y;
	Pos(short x, short y);
	~Pos() = default;
};

Pos::Pos(short px, short py) :
_pos{ 0 },
x{ (short*)(&_pos)[0] },
y{ (short*)(&_pos)[1] }
{
	*x = px;
	*y = py;
}

class Chunk
{
public:
	const static int width = 4;
	const static int height = 4;
	const static int res = 4;
	const static int total = width*height*res;
	typedef	std::bitset<total>	t_data;
	typedef	std::bitset<res>	t_item;
	//requires t_data has std::hash
	std::hash<t_data>			hash;
	std::string	serialize();
	char cell(int x, int y);
	Chunk();
	Chunk(const t_data& pdata);
	Chunk(const std::string& hex);
	~Chunk() = default;
private:
	t_data* data;
	Chunk(const Chunk& c) = delete;
	void operator=(const Chunk& c) = delete;
};

Chunk::Chunk()// : data {  } //<- std::unique_ptr<std::bitset<total>>(new std::bitset<total>())
{
	data = new t_data();
}

//with res=4, w=4, h=4, a 64 bit hex can store 16 types (0-F) in a 4x4 grid:
//0x FFFF FFFF FFFF FFFF for a total of 16 tiles.
std::string Chunk::serialize()
{
	std::stringstream ss;
	ss << std::hex << std::uppercase << data->to_ullong();
	return ss.str();
}
/*
Chunk::Chunk(const t_data& pdata) : Chunk()
{
	*data &= pdata;
}*/

Chunk::Chunk(const std::string& hex)
{
	std::stringstream ss;
	ss << std::hex << hex;
	unsigned long long n;
	ss >> n;
	data = new t_data(n);
}

/**Go ahead and use only the first parameter for 1d reference. */
char Chunk::cell(int x, int y=0)
{
	//result initializes a new t_item as a subset of data.
	//right now, it's a bitset created from a bitset.
	//bitset's most significant bits are at the end. Go backwards
	t_item set = t_item();
	for(int i = 3; i>=0; i--)
		set.set( i, data->test(total - 1 - (i + x*res + y*width*res))  );
	return (char)set.to_ulong();
}


int main(int argc, char* argv[])
{
	typedef Tool_Configurable t_config;

		SDL_Init(SDL_INIT_EVERYTHING);

	auto config = unique_ptr<t_config>{new t_config("test.info")};
	std::string data = config->serialize();

		SDL_Renderer* ren;
		SDL_Window* win;

		SDL_CreateWindowAndRenderer(600, 600, 0, &win, &ren);

	Tool_Asset* asset = new Tool_Asset{ "fire_fighter.png", ren };
	asset->draw(10, 10);
	delete asset;

		UDPpacket* packet = SDLNet_AllocPacket(512);
	auto messenger = new Tool_Messenger();
	
	messenger->write(data, packet->data);
		SDL_ShowSimpleMessageBox(0, "test", messenger->read(packet->data).c_str(), NULL);
	
		SDLNet_FreePacket(packet);
	delete messenger;

	auto c = unique_ptr<Chunk> { new Chunk("FFFF0F0102159180") };

	SDL_RenderPresent(ren);
	SDL_Delay(2000);

	SDL_Quit();
	return 1;
}