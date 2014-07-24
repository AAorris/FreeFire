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

	if (!SDL_WasInit(0))
		SDL_Init(SDL_INIT_EVERYTHING);

	auto gfx = wrap(new _gfx{});
	auto sim = Facet_Sim{};


	_cfg sessionConfig = _cfg{ "config.INFO" };
	sim.connect(sessionConfig);
	gfx->connect(sessionConfig);
	//init map
	for (auto item : sessionConfig->get_child("Map"))
	{

		char key = item.first.front();
		scalar pos = scalar{ item.second.data() };
		sim.set(pos, key);
	}
	//std::map<char, const Tool_Data> templates;
	/*std::vector<Tool_Data> dataContainer;

	DataTranslator tr{};
	for (auto item : sessionConfig->get_child("Templates"))
	{
		std::string group = item.first;
		auto key = item.second.get_value<char>();
		sim.templates.insert(std::make_pair(
			key,
			create_data(item.second, tr, scalar(0, 0))
			));
	}

	for (auto item : sessionConfig->get_child("Map"))
	{
		char key = item.first.front();
		scalar pos = scalar{ item.second.data() };
		auto newData = sim.set(pos, key);
		gfx->connect(&newData);
	}*/

	auto keys = SDL_GetKeyboardState(NULL);
	int mousex = 0;
	int mousey = 0;
	SDL_GetMouseState(&mousex,&mousey);
	scalar v_camera{ 0, 0 };
	double v_zoom = 0;
	bool playing = true;
	SDL_Event e;

	const int size = sessionConfig->get_optional<int>("Settings.worldSize").get_value_or(100);
	const double coverage = sessionConfig->get_optional<double>("Settings.treeCoverage").get_value_or(0.5);
	for (int i = 0; i < int(size*size*coverage); i++)
		sim.set(scalar(rand() % size - size / 2, rand() % size - size / 2), '3');//put(AA::Pos(rand() % size - size / 2, rand() % size - size/2), '3');
	for (int i = 0; i < int(size*size*coverage); i++)
		sim.set(scalar(rand() % size - size / 2, rand() % size - size / 2), '4');

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
			if (e.type == SDL_WINDOWEVENT)
			{
				if (e.window.type == SDL_WINDOWEVENT_RESIZED)
				{
					gfx->resize(e.window.data1, e.window.data2);
				}
			}
			if (e.type == SDL_MOUSEWHEEL)
			{
				if (abs(e.wheel.y) > 0)
					v_zoom += 0.03*e.wheel.y;
			}
			
			if (e.type == SDL_MULTIGESTURE)
			{
				SDL_MultiGestureEvent& g = e.mgesture;
				if (g.numFingers == 1)
				{
				}
				if (g.numFingers == 2)
					v_zoom += 0.3*g.dDist;
			}
			if (e.type == SDL_FINGERMOTION)
			{
				SDL_TouchFingerEvent& f = e.tfinger;
				v_camera.x = -f.dx*1024;
				v_camera.y = -f.dy*768;
				//gfx->moveCamera(scalar(-f.dx*100, -f.dy*100));
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
		}
		
		if (v_camera1 == v_camera)
			v_camera *= 0.9;

		gfx->zoomCamera(v_zoom);

		v_zoom *= 0.9;
		gfx->clear();

		//sim->put(AA::Pos(rand() % size - size / 2, rand() % size - size / 2), 'F');

		auto& set = sim.data;

		//draw
		gfx->draw(sim.data);

		gfx->present();

		long ticks = SDL_GetTicks();
		sim.update(16);
		long ticks2 = SDL_GetTicks();
		int delay = 16 - (ticks2 - ticks);
		if (delay > 0) SDL_Delay(delay);
	}

//	gfx->draw(assets[0]);
//	gfx->draw(assets[2],32,32);
}
