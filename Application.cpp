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
	scalar v_camera{ 0, 0 };
	double v_zoom = 0;
	bool playing = true;

	while (SDL_QuitRequested() == false && playing)
	{
		keys = SDL_GetKeyboardState(NULL);

		if (keys[SDL_SCANCODE_A])
			v_camera.x -= 0.01;
		if (keys[SDL_SCANCODE_D])
			v_camera.x += 0.01;
		if (keys[SDL_SCANCODE_W])
			v_camera.y -= 0.01;
		if (keys[SDL_SCANCODE_S])
			v_camera.y += 0.01;
		if (keys[SDL_SCANCODE_KP_PLUS]){
			v_zoom += 0.02;
		}
		if (keys[SDL_SCANCODE_KP_MINUS]){
			v_zoom -= 0.02;
		}
		if (keys[SDL_SCANCODE_ESCAPE])
			playing = false;

		gfx->moveCamera(v_camera);
		gfx->zoomCamera(v_zoom);
		v_camera *= 0.975;
		v_zoom *= 0.975;

		sim->put(AA::Pos(0, 0), 'F');
		sim->put(AA::Pos(rand() % 10, rand() % 10), 'F');
		gfx->clear();
		for (auto& item : sim->getMap())
		{
			auto location = item.first;
			auto& value = item.second;
			gfx->draw(location, value);
		}
		gfx->present();
		sim->update();
		SDL_Delay(16);
	}

//	gfx->draw(assets[0]);
//	gfx->draw(assets[2],32,32);
}
