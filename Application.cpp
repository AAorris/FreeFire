#include "stdafx.h"
#include "Application.h"

#include <SDL2\SDL_net.h>
#include <bitset>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/property_tree/ptree.hpp>
using PT = boost::property_tree::ptree;

#include "Tool_Configurable.h"
#include "Tool_Asset.h"
#include "Tool_Messenger.h"
#include "Tool_Pos.h"

#include "Module.h"
#include "Module_Fire.h"

#include "Facet_Sim.h"
#include "Facet_Gfx.h"
#include "scalar.h"


Application::Application()
{
}


Application::~Application()
{
}

void Application::run()
{
	using v2 = AA::Pos;

#ifndef SDLINIT
#define SDLINIT
	SDL_Init(SDL_INIT_EVERYTHING);
#endif

	auto gfx = wrap(new _gfx{});
	auto sim = wrap(new _sim{});


	auto config = wrap(new Tool_Configurable("config.INFO"));
	auto assets = config->getAssets();
	for (auto it = begin(assets); it != end(assets); ++it) {
		auto key = it->at(0);
		*it = std::move(it->substr(1, it->size() - 1)); //cut the first character out
		gfx->loadAsset(*it,key);
	}

	std::string mapPath = config->get("Map");
	
	sim->loadState(mapPath);

	auto keys = SDL_GetKeyboardState(NULL);
	int mousex = 0;
	int mousey = 0;
	SDL_GetMouseState(&mousex,&mousey);
	scalar v_camera{ 0, 0 };
	double v_zoom = 0;
	bool playing = true;
	SDL_Event e;

	const int size = 100;
	const double coverage = 1.0;
	for (int i = 0; i < int(size*size*coverage); i++)
		sim->put(AA::Pos(rand() % size - size / 2, rand() % size - size/2), '3');
	for (int i = 0; i < int(size*size*coverage); i++)
		sim->put(AA::Pos(rand() % size - size / 2, rand() % size - size/2), '4');

	while (SDL_QuitRequested() == false && playing)
	{
		auto mouse = SDL_GetMouseState(&mousex, &mousey);
		keys = SDL_GetKeyboardState(NULL);

		auto v_camera1 = v_camera;
		if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])
			v_camera.x -= 1;
		if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT])
			v_camera.x += 1;
		if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])
			v_camera.y -= 1;
		if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])
			v_camera.y += 1;
		if (keys[SDL_SCANCODE_KP_PLUS])
			v_zoom += 0.005;
		if (keys[SDL_SCANCODE_KP_MINUS])
			v_zoom -= 0.005;
		if (keys[SDL_SCANCODE_ESCAPE])
			playing = false;

		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_MOUSEWHEEL)
			{
				if (abs(e.wheel.y) > 0)
					v_zoom += 0.03*e.wheel.y;
			}

			if (e.type == SDL_MOUSEMOTION)
			{
				if (mouse&SDL_BUTTON_MMASK)
				{
					v_camera = scalar(-e.motion.xrel, -e.motion.yrel);
					gfx->moveCamera( scalar(-e.motion.xrel, -e.motion.yrel) );
				}
			}

			if (e.type == SDL_KEYUP)
			{
			}
		}

		if ((mouse&SDL_BUTTON_MMASK)==0){
			gfx->moveCamera(v_camera);
			if (v_camera1 == v_camera)
				v_camera *= 0.9;
		}

		gfx->zoomCamera(v_zoom);

		v_zoom *= 0.9;
		gfx->clear();

		sim->put(AA::Pos(rand() % size - size / 2, rand() % size - size / 2), 'F');

		auto set = sim->getMap();
		
		for (auto& item : *set)
		{
			auto location = item.first;
			auto& value = item.second;
			gfx->draw(location, value);
		}

		gfx->present();

		long ticks = SDL_GetTicks();
		sim->update();
		long ticks2 = SDL_GetTicks();
		int delay = 16 - (ticks2 - ticks);
		if (delay > 0) SDL_Delay(delay);
	}

//	gfx->draw(assets[0]);
//	gfx->draw(assets[2],32,32);
}
