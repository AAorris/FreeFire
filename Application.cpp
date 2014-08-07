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
#include <SDL2\SDL_ttf.h>

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
	if (!TTF_WasInit())
		TTF_Init();

	auto gfx = wrap( new _gfx(scalar(98,90),true) );
	auto sim = Facet_Sim{};

	SDL_FlushEvents(0, UINT_MAX);

	_cfg sessionConfig = _cfg{ "config.INFO" };
	sim.connect(sessionConfig);
	gfx->connect(sessionConfig);


	_cfg newSession = _cfg{ "assets/Session.INFO" };
	auto a = newSession.getData();
	for (auto item : newSession->get_child("Config.UI"))
	{
		if (item.second.get<std::string>("Type") != "Menu")
			activeUIs.push_back(new UI(gfx->context(), item.second));
	}

	//init map
	for (auto item : sessionConfig->get_child("Map"))
	{

		char key = item.first.front();
		scalar pos = scalar{ item.second.data() };
		sim.set(pos, key);
	}

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
			v_camera.x += 1;
		if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT])
			v_camera.x -= 1;
		if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])
			v_camera.y += 1;
		if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])
			v_camera.y -= 1;
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
				if (ui->type == "Menu")
				{
					//ui->isAlive(false);
					hasMenu = true;
				}
			}
			if (!hasMenu && sim.selectedUnit!=nullptr)
			{
				auto uicfg = boost::property_tree::ptree(newSession->get_child("Config.UI.SelectionMenu"));
				auto unitcfg = sim.selectedUnit->root->properties;
				auto area = boost::property_tree::ptree();
				area.put<int>("w", uicfg.get<int>("Background.Area.w"));
				area.put<int>("h", uicfg.get<int>("Background.Area.h"));
				area.put<int>("x", mousex + uicfg.get<int>("Background.Area.x"));
				area.put<int>("y", mousey + uicfg.get<int>("Background.Area.y"));
				uicfg.put_child("Background.Area", area);
				activeUIs.push_back(new UI(gfx->context(), uicfg));
			}
		}

		if (rightMouseReleased && sim.selectedUnit != NULL){
			scalar dest = scalar::round(gfx->getCell(scalar(mousex, mousey)));
			sim.selectedUnit->destination = dest;
		}

		if (sim.selectedUnit != NULL)
		{
			gfx->highlightCell(sim.selectedUnit->position);
			if (sim.selectedUnit->destination.is_initialized())
				gfx->highlightCell(sim.selectedUnit->destination.get());
		}

		using boost::property_tree::ptree;
		ptree newData{};

		ptree wind{};
		wind.put<int>("N", sim.wind("N"));
		wind.put<int>("S", sim.wind("S"));
		wind.put<int>("E", sim.wind("E"));
		wind.put<int>("W", sim.wind("W"));

		ptree mouseData{};
		mouseData.put<int>("x", mousex);
		mouseData.put<int>("y", mousey);
		int leftState = leftMouseReleased ? 2 : (mouse&SDL_BUTTON_LEFT) ? 1 : 0;
		mouseData.put<int>("left", leftState);
		int rightState = rightMouseReleased ? 2 : (mouse&SDL_BUTTON_RIGHT) ? 1 : 0;
		mouseData.put<int>("right", rightState);
		int middleState = (mouse&SDL_BUTTON_MIDDLE) ? 1 : 0;
		mouseData.put<int>("middle", middleState);

		newData.put_child("Wind",wind);
		newData.put_child("Mouse", mouseData);
		newData.put<int>("Incidents", sim.information.get_optional<int>("Incidents").get_value_or(0));
		//element->update(&newData);
		//element->draw();

		//std::vector<std::vector<UI*>::iterator> toRemove;
		if (gfx->getZoom() <= 1) {
			for (auto& item : activeUIs)
			{
				item->update(&newData);
				item->draw();
			}
			activeUIs = std::vector<UI*>(activeUIs.begin(), std::remove_if(begin(activeUIs), end(activeUIs), [](UI* i){ return !i->isAlive(); }));
		}

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
