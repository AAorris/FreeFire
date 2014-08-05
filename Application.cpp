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

#include "Tool_UIElement.h"

Application::Application()
{
}


Application::~Application()
{
}

void Application::run()
{
	using v2 = AA::Pos;
	activeUIs = std::vector<UI*>();

	if (!SDL_WasInit(0))
		SDL_Init(SDL_INIT_EVERYTHING);

	auto gfx = wrap( new _gfx(scalar(1024,768)) );
	auto sim = Facet_Sim{};


	_cfg sessionConfig = _cfg{ "config.INFO" };
	sim.connect(sessionConfig);
	gfx->connect(sessionConfig);


	_cfg newSession = _cfg{ "assets/Session.INFO" };
	auto a = newSession.getData();
	for (auto item : newSession->get_child("Config.UI"))
	{
		activeUIs.push_back(new UI(gfx->context(), item.second));
	}
	//auto cfg = newSession->get_child("Config.UI.Placeholder");
	//auto d = newSession.getData("Config.UI.Placeholder");
	//UI::art::context ctx = gfx->context();
	//UI* element = new UI(ctx, cfg);

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

		bool leftMouseReleased = false;
		bool rightMouseReleased = false;

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
					v_camera = scalar(e.motion.xrel, e.motion.yrel);
					gfx->moveCamera( v_camera );
				}
			}

			if (e.type == SDL_KEYUP)
			{
			}

			if (e.type == SDL_MOUSEBUTTONUP)
			{
				if (e.button.button == SDL_BUTTON_LEFT)
					leftMouseReleased = true;
				if (e.button.button == SDL_BUTTON_RIGHT)
					rightMouseReleased = true;
			}
		}

		if ((mouse&SDL_BUTTON_LMASK) == 0){
			//leftMouseReleased = true;
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

		gfx->draw(sim.data);

		if (leftMouseReleased)
		{
			auto cell = gfx->getCell(scalar(mousex, mousey));
			gfx->highlightCell(cell);
			sim.select(cell);

			bool hasMenu = false;
			for (auto ui : activeUIs)
			{
				if (ui->getType() == "Menu")
				{
					hasMenu = true;
				}
			}

			if (!hasMenu)
			{
				auto uicfg = boost::property_tree::ptree(newSession->get_child("Config.UI.InfoMenu"));
				auto area = boost::property_tree::ptree();
				area.put<int>("w", uicfg.get<int>("Background.Area.w"));
				area.put<int>("h", uicfg.get<int>("Background.Area.h"));
				area.put<int>("x", mousex);
				area.put<int>("y", mousey);
				uicfg.put_child("Background.Area", area);
				activeUIs.push_back(new UI(gfx->context(), uicfg));
			}
		}

		if (rightMouseReleased && sim.selectedUnit != NULL)
			sim.selectedUnit->destination = gfx->getCell(scalar(mousex, mousey));

		if (sim.selectedUnit != NULL)
		{
			gfx->highlightCell(sim.selectedUnit->position);
			if (sim.selectedUnit->destination.is_initialized())
				gfx->highlightCell(sim.selectedUnit->destination.get());
		}

		boost::property_tree::ptree newData{};
		boost::property_tree::ptree wind{};
		wind.put<int>("N", rand() % 100);
		wind.put<int>("S", rand() % 100);
		wind.put<int>("E", rand() % 100);
		wind.put<int>("W", rand() % 100);
		newData.put_child("Wind",wind);
		//element->update(&newData);
		//element->draw();

		//std::vector<std::vector<UI*>::iterator> toRemove;
		for (auto& item : activeUIs)
		{
			item->update(&newData);
			item->draw();
		}
		std::remove_if(begin(activeUIs), end(activeUIs), [](UI* i){ return !i->isAlive(); });

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
