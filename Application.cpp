#include "stdafx.h"
#include "Application.h"

#include <SDL2\SDL_net.h>
#include <bitset>
#include <fstream>
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


Application::Application()
{
}


Application::~Application()
{
}

void Application::run()
{
	auto gfx = wrap(new _gfx{});
	auto sim = wrap(new _sim{});

	auto config = wrap(new Tool_Configurable("config.INFO"));
	auto assets = config->getAssets();
	for (auto a : assets)
		gfx->loadAsset(a);

	std::string mapPath = config->get("Map");
	sim->loadState(mapPath);
	gfx->drawChunk(sim,AA::Pos(1,1));

	gfx->draw(assets[0]);
	gfx->present();
}
