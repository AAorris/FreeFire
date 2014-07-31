#pragma once
#include "stdafx.h"

#define DISABLE 0
#define ENABLE 1
#define TEST 2

#define CONFIG		ENABLE
#define NET			DISABLE
#define ASSETS		ENABLE
#define GFX			ENABLE
#define FIRE		ENABLE
#define CHUNKS		DISABLE

#define APPLICATION TEST

#include "Tool_Configurable.h"
#include "Tool_Asset.h"
#include "Tool_Messenger.h"
#include "Tool_Pos.h"
#include "Tool_Data.h"
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
#include <string>

#include "SDLButton.h"
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
#if APPLICATION == TEST

	{
		auto button = wrap(new SDLButton(0, 0, 600, 300, "Start"));
		auto g = wrap(new _gfx(scalar(600, 300)));
		g->loadAsset("assets/loading.png", 1);
		auto bg = g->getAsset(1);
		while (button->clicking == false && !SDL_QuitRequested())
		{
			bg->draw(0, 0);
			button->update();
			g->draw(button.get());
			g->present();
			
			SDL_Delay(16);
		}
	}

	auto app = wrap(new Application());
	app->run();


#endif // !_DEBUG

	/*---------------------
	UNIT TESTS
	---------------------*/
	if (!SDL_WasInit(0))
		SDL_Init(SDL_INIT_EVERYTHING);
	/*---------------------
	UNIT TEST DEFINITIONS
	---------------------*/

	auto test = [](bool(*func)(), std::string name) {
		SDL_Log("Testing %s\n", name.c_str());
		bool result = func();
		SDL_Log("Testing %s ", name.c_str());
		SDL_Log((result ? "Succeeded\n" : "Failed\n"));
	};

#if CONFIG==TEST
	auto testConfig = [](){
		auto config = t_config("test.info");
		std::string data = config.getData();
		return true;
	};
	test(testConfig, "Config Test");
#endif
#if NET==TEST
	auto testNetMessenger = [](){
		UDPpacket* packet = SDLNet_AllocPacket(512);
		auto messenger = wrap(new Tool_Messenger());

		auto data = "Hello, world!";
		messenger->write(data, packet->data);
		SDL_ShowSimpleMessageBox(0, "test", messenger->read(packet->data).c_str(), NULL);
		SDLNet_FreePacket(packet);
	};
#endif
#if ASSETS==TEST
	auto testAssets = [](){
		SDL_Renderer* ren;
		SDL_Window* win;
		SDL_CreateWindowAndRenderer(600, 600, 0, &win, &ren);
		auto asset = wrap(new Tool_Asset{ "fire_fighter.png", ren });
		asset->draw(10, 10);
		SDL_RenderPresent(ren);
		SDL_Delay(2000);
		return true;
//		int index = _asset::useLookup("fire_fighter.png");
//		SDL_Log("\nIndex was %d\n\n", index);
//		return index!=-1;
	};
	test(testAssets, "Asset Test");
#endif
#if GFX==TEST
	auto testGFX = []() {
		auto gfx = wrap(new _gfx());
		auto path = "fire_fighter.png";
		int id = 0;
		gfx->loadAsset(path, id);
		gfx->draw(id,scalar(100,100));
		gfx->present();
		return true;
	};
	test(testGFX, "GFX Test");
#endif
#if FIRE==TEST
	auto testFire = [](){
		//using AA::Pos;

		auto sim = Facet_Sim{};
		auto gfx = wrap(new _gfx{});
		
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
		int time = 0;
		while (time < 10000 && !SDL_QuitRequested())
		{
			//update
			sim.update(16);
			//draw
			gfx->draw(sim.data);
			SDL_Delay(16);
			gfx->present();
			time += 16;
		}
		SDL_Delay(3000);

		return true;
	};
	test(testFire,"Fire");
#endif
#if CHUNKS==TEST
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
	test(testChunks, "Chunk Test");
#endif

	SDL_Delay(1000);

	logFile.close();
	if(SDL_WasInit(0))
		SDL_Quit();
	return 1;
}