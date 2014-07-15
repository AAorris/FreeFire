#pragma once
#include "stdafx.h"
//#include "Role.h"
#include "Tool_Configurable.h"
#include "Tool_Asset.h"
#include "Tool_Messenger.h"
#include "Tool_Pos.h"
#include "Module_Fire.h"
#include "Facet_Sim.h"
#include "Facet_Gfx.h"
#include "Module.h"
#include "Application.h"
#include <SDL2\SDL_net.h>
#include <bitset>
#include <fstream>
#include <cassert>
#include <boost/range/adaptor/reversed.hpp>

//#include <sstream>

using boost::adaptors::reverse;
std::ofstream logFile;

void fflogger(void* userdata, int category, SDL_LogPriority priority, const char* message)
{
	// Change to fit your needs (like to output to a file) 
	logFile << message;
}

int main(int argc, char* argv[])
{
	/*---------------------
	GLOBAL INIT
	---------------------*/
	SDL_LogSetOutputFunction(&fflogger, NULL);
	//SDL_LogSetOutputFunction(ffloger,)
	typedef Tool_Configurable t_config;
	logFile.open("log.txt");

	/*---------------------
	Main
	---------------------*/

	auto app = wrap(new Application());
	app->run();

	/*---------------------
	UNIT TESTS
	---------------------*/

#define MAINTESTS
#ifdef MAINTESTS
#ifndef SDLINIT
#define SDLINIT
	SDL_Init(SDL_INIT_EVERYTHING);
#endif
#endif

	/*---------------------
	UNIT TEST DEFINITIONS
	---------------------*/
	auto testConfig = [](){
		auto config = wrap(new t_config("test.info"));
		std::string data = config->serialize();
	};

	auto testNetMessenger = [](){
		UDPpacket* packet = SDLNet_AllocPacket(512);
		auto messenger = wrap(new Tool_Messenger());

		auto data = "Hello, world!";
		messenger->write(data, packet->data);
		SDL_ShowSimpleMessageBox(0, "test", messenger->read(packet->data).c_str(), NULL);
		SDLNet_FreePacket(packet);
	};

	auto testAssets = [](){
		SDL_Renderer* ren;
		SDL_Window* win;
		SDL_CreateWindowAndRenderer(600, 600, 0, &win, &ren);
		auto asset = wrap(new Tool_Asset{ "fire_fighter.png", ren });
		asset->draw(10, 10);
		SDL_RenderPresent(ren);
		SDL_Delay(2000);

//		int index = _asset::useLookup("fire_fighter.png");
//		SDL_Log("\nIndex was %d\n\n", index);
//		return index!=-1;
	};

	auto testGFX = []() {
		auto gfx = wrap(new _gfx());
		auto path = "fire_fighter.png";
		int id = 0;
		gfx->loadAsset(path,++id);
		gfx->draw(path);
		gfx->present();
		return true;
	};

	auto testChunks = []() {
		auto sim = wrap(new t_sim());
		using AA::Pos;
		//sim->put(Pos(1, 0), 0xA);
		//sim->put(Pos(2, 0), 0xC);
		//sim->put(Pos(3, 3), 0xB);
		sim->putChunk("0110110100110100", Pos(0, 0));
		std::string cs = sim->getChunk(Pos(0, 0));
		sim->putChunk(cs, Pos(1,0));
		std::string cs2 = sim->getChunk(Pos(1, 0));
		assert(cs2 == cs);
		return true;
	};

	auto testFire = [](){
		using AA::Pos;

		SDL_Renderer* ren;
		SDL_Window* win;
		SDL_CreateWindowAndRenderer(600, 600, 0, &win, &ren);

		auto a_fire = wrap(new Tool_Asset{ "fire.png", ren });
		auto fireModule = Module_Fire{};
		auto sim = wrap(new Facet_Sim{});
		sim->putChunk("F000 F000 F000 F000", Pos(0, 0));
		sim->putChunk("F000 F000 F000 F000", Pos(1, 0));
		sim->putChunk("F000 F000 F000 F000", Pos(1, 1));
		sim->putChunk("F000 F000 F000 F000", Pos(0, 1));

		auto draw = [&sim, &ren, &a_fire]() {
			SDL_RenderClear(ren);
			
			SDL_RenderPresent(ren);
		}; // testfire draw

//		fireModule.sync(sim->getMap);
		sim->saveState("autosave.map");
		fireModule.loadState("autosave.map");
		int time = 0;
		while (time < 1000 * 100)
		{
			fireModule.update(100);
			time += 100;

			auto news = fireModule.getNews();
			for (auto n : news)
			{
				SDL_Log(FF::translate<Module_Fire, t_sim>()(n, sim.get()).c_str());
			} if (news.size() > 0) {
				std::stringstream name;
				name << "Map" << time << ".map";
				sim->saveState(name.str());
			}

			draw();

			SDL_Delay(16);
		}

		return true;
	}; // testfire

	auto test = [](bool(*func)(), std::string name) {
		SDL_Log("Testing %s\n", name.c_str());
		bool result = func();
		SDL_Log("Testing %s ", name.c_str());
		SDL_Log((result ? "Succeeded\n" : "Failed\n"));
	}; 

	/*---------------------
	MAIN
	---------------------*/

	//testConfig();
	//testNetMessenger();
	//test(testAssets, "Asset Test");
	//testChunks();
	//test(testFire,std::string("Fire"));
	//test(testGFX, "GFX Test");

	SDL_Delay(1000);

	logFile.close();
#ifdef SDLINIT
#undef SDLINIT
	SDL_Quit();
#endif
	return 1;
}