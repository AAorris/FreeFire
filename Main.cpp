#pragma once
#include "stdafx.h"
#include <SDL2\SDL.h>
#include "Role.h"
#include "Tool_Configurable.h"
#include <sstream>

int main(int argc, char* argv[])
{
	typedef Tool_Configurable t_config;
	using std::unique_ptr;

	SDL_Init(SDL_INIT_EVERYTHING);

	auto config = unique_ptr<t_config>{new t_config("test.info")};
	std::string data = config->serialize();
	SDL_ShowSimpleMessageBox(0, "test", data.c_str(), NULL);

	SDL_Quit();
	return 1;
}