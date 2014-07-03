#pragma once
#include "stdafx.h"
//#include "Role.h"
#include "Tool_Configurable.h"
#include "Tool_Asset.h"
#include "Tool_Messenger.h"
#include <SDL2\SDL_net.h>
//#include <sstream>
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

	SDL_RenderPresent(ren);
	SDL_Delay(2000);

	SDL_Quit();
	return 1;
}