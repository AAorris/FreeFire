#include "stdafx.h"
#include "Application.h"

#include <SDL2\SDL_net.h>
#include <bitset>
#include <fstream>
#include <algorithm>
#include <random>
#include <cassert>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost\property_tree\info_parser.hpp>
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

#include "Geometry.h"

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
	uis = Facet_UI();
	//activeUIs = std::vector<UI*>();

	if (!SDL_WasInit(0))
		SDL_Init(SDL_INIT_EVERYTHING);
	if (!TTF_WasInit())
		TTF_Init();
	tile::Fire::initFire();

	auto gfx = wrap( new _gfx(scalar(98,90),true) );
	auto sim = Facet_Sim{};

	SDL_FlushEvents(0, UINT_MAX);

	_cfg sessionConfig = _cfg{ "config.INFO" };
	sim.connect(sessionConfig);
	gfx->connect(sessionConfig);
	uis.connect(sessionConfig);

	_cfg newSession = _cfg{ "assets/Session.INFO" };
	auto a = newSession.getData();
	auto items = newSession->get_child_optional("Config.UI");
	if (items.is_initialized()){
		for (auto item : items.get())
		{
			if (item.second.get<std::string>("Type") != "Menu")
				uis.elements.push_back(new UI(gfx->context(), item.second));
		}
	}
	else {
		SDL_ShowSimpleMessageBox(0,"Configuration problem", "Couldn't find ui items...", NULL);
	}

	//init map
	try {
		for (auto item : sessionConfig->get_child("Map"))
		{
			char key = item.first.front();
			scalar pos = scalar{ item.second.data() };
			sim.insert(pos, key);
		}
	}
	catch (std::exception e)
	{
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


	PerlinNoise pn(time(NULL));
	for (int i = 0; i < size*size; i++)
	{
		int x = i % size - size/2;
		int y = i / size - size/2;
		double noise = pn.noise(x/128.0 * 5, y/128.0 * 6, 0);
		char elevation = ((noise-0.5) * 255);
		elevation = elevation*0.6 + 0 * 0.1;
		elevation = (elevation>127) ? 127 : elevation;

		auto& land = sim.data[tile::GEOGRAPHYGROUP];
		auto temp = tile::Data(nullptr, scalar(x, y));
		auto newLand = new tile::Land(elevation, rand() % 255, std::move(temp));
		facet::insert(scalar(x, y), newLand, &land);
		//land.insert(std::make_pair(scalar(x,y),dynamic_cast<tile::Data*>(newLand)));
		
		if (newLand->isWater() == false && newLand->elevation < 32 && rand()%100 < newLand->treeChance()*100)
			sim.insert(scalar(x, y), (rand() % 100 > 50) ? '3' : '4');
	}

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
				v_camera.x = f.dx*1024;
				v_camera.y = f.dy*768;
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


		if (leftMouseReleased)
		{
			auto cell = gfx->getCell(scalar(mousex, mousey));
			gfx->highlightCell(cell);
			sim.select(cell);

			bool hasMenu = false;
			for (auto ui : uis.elements)
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
				auto abilities = boost::property_tree::ptree();
				for (auto i = unitcfg.begin(); i != unitcfg.end(); i++)
					abilities.put(i->first.data(),i->first.data());

				std::ostringstream oss;
				boost::property_tree::write_info(oss, abilities);
				auto str = oss.str();

				area.put<int>("w", uicfg.get<int>("Background.Area.w"));
				area.put<int>("h", uicfg.get<int>("Background.Area.h"));
				area.put<int>("x", mousex + uicfg.get<int>("Background.Area.x"));
				area.put<int>("y", mousey + uicfg.get<int>("Background.Area.y"));
				uicfg.put_child("Background.Area", area);
				uicfg.put_child("Abilities", abilities);
				uicfg.put("Type", "Menu");
				uis.elements.push_back(new UI(gfx->context(), uicfg));
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

		sim.information.put_child("Wind",wind);
		sim.information.put_child("Mouse", mouseData);
		sim.information.put<int>("Incidents", sim.information.get_optional<int>("Incidents").get_value_or(0));
		//element->update(&newData);
		//element->draw();

		//std::vector<std::vector<UI*>::iterator> toRemove;
		if (gfx->getZoom() <= 1) {
			gfx->drawOverview(sim.data);
		}
		else {
			gfx->draw(sim.data);
		}

		for (auto& item : uis.elements)
			item->draw();
		uis.elements = std::vector<UI*>(uis.elements.begin(), std::remove_if(begin(uis.elements), end(uis.elements), [](UI* i){ return !i->isAlive(); }));

		uis.update(&sim.information, 16);

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
