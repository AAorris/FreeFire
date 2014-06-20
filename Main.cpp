#include <SDL2\SDL_main.h>
#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>

#include <SDL2\SDL.h>
#include <SDL2\SDL_image.h>
#include <SDL2\SDL_ttf.h>
#include <SDL2\SDL_net.h>

#include "pos.h"

#include "UDPClient.h"
#include "UDPServer.h"

#include "FF_Packets.h"
#include "GFX.h"
#include "sprite.h"

#define ROOT std::string("./")

//globals
enum RUN_TYPE {	NONE, SERVER, CLIENT, MODERATOR, TEAM_BOARD };
int finding_runtype = 1;
int running = 1;
RUN_TYPE type;

void handleEvents(int& running);

int thread_net(void* data);
int thread_sim(void* data);

int runClient();
int runServer();
int runModerator();
int runTeamBoard();


int main(int argc, char* argv[])
{
	/*=============================================================
	===============================================================
						    Type definitions
	===============================================================
	==============================================================*/


	/*=============================================================
	===============================================================
						    Initialization
	===============================================================
	==============================================================*/

	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF);
	TTF_Init();
	SDLNet_Init();
	int treturn = 0;
	GFX menu1;
	menu1.init("Free Fire Menu",600,300,true);

	/*=============================================================
	===============================================================
						    Start running
	===============================================================
	==============================================================*/


	menu1.loadAsset("menubg", ROOT+"assets/menu2.png");

	int f[4] = {3,4,5,6};
	std::vector<int> frames (f, f+sizeof(f)/sizeof(f[0]));
	sprite spindleSprite = sprite(ROOT+"assets/spindle.png",72,73);
	menu1.loadSprite(spindleSprite);
	spindleSprite.addAnimation("run",frames,5);
	spindleSprite.play("run");

	SDLButton testButton = SDLButton();
	testButton.x = 200;
	testButton.y = 100;
	testButton.w = 100;
	testButton.h = 50;
	testButton.textLabel = "Client";

	int y = 80;
	int padding = 5;
	int h = 30;
	int x = 340;
	SDLButton clientButton = SDLButton(x, y, 150, 30, "Client"); y += h + padding;
	SDLButton serverButton = SDLButton(x, y, 150, 30, "Server"); y += h + padding;
	SDLButton boardButton = SDLButton(x, y, 150, 30, "Team Board"); y += h + padding;
	SDLButton modButton = SDLButton(x, y, 150, 30, "Moderator"); y += h + padding;

	/*=============================================================
	===============================================================
						Main Thread Main Loop
	===============================================================
	==============================================================*/

	while(finding_runtype) {

		//handle events
		handleEvents(running);
		if (!running) finding_runtype = false;

		//clear the screen for drawing
		menu1.fill("menubg");

		//Update each button to check for mouse input
		clientButton.update();
		serverButton.update();
		boardButton.update();
		modButton.update();

		//With the right mouse input on these buttons,
		//make a runtime decision
		if (clientButton.clicking){
			type = RUN_TYPE::CLIENT;
			finding_runtype = false;
		}
		if (serverButton.clicking){
			type = RUN_TYPE::SERVER;
			finding_runtype = false;
		}
		if (boardButton.clicking){
			type = RUN_TYPE::TEAM_BOARD;
			finding_runtype = false;
		}
		if (modButton.clicking){
			type = RUN_TYPE::MODERATOR;
			finding_runtype = false;
		}

		//Draw each button
		menu1.drawButton(clientButton);
		menu1.drawButton(serverButton);
		menu1.drawButton(boardButton);
		menu1.drawButton(modButton);

		//Draw each sprite
		menu1.drawSprite(spindleSprite,10,10);
		spindleSprite.update(1000 / 60);

		//Show the window
		menu1.show();

		SDL_Delay(1000/64);
	}
	menu1.cleanup();
	
	if (type == CLIENT)
		runClient();
	if (type == SERVER)
		runServer();
	if (type == TEAM_BOARD)
		runTeamBoard();
	if (type == MODERATOR)
		runModerator();

	/*=============================================================
	===============================================================
							    Clean Up
	===============================================================
	==============================================================*/

	SDLNet_Quit();
	IMG_Quit();
	SDL_Quit();
	return -1;
}


void handleEvents(int& running)
{
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_WINDOWEVENT)
		{
			switch (e.window.event) {
			case SDL_WINDOWEVENT_CLOSE:
				running = false;
				break;
			default:
				break;
			}
		}
	}
}

int runClient()
{
	SDL_Thread* net = SDL_CreateThread(thread_net, "NetClientFF", (void*)RUN_TYPE::CLIENT);
	int netstatus;

	std::ofstream log;
	log.open("Client_MainLog.txt");
	log << "Opening Client log...\n";
	GFX client = GFX();
	client.init("FreeFire Client", 1024, 768);
	while (running)
	{
		client.fill(255, 255, 255, 255);
		client.write("Hello, Client!", 10, 10);
		client.show();
		log << "Handling events...\n";
		handleEvents(running);
		SDL_Delay(16);
	}
	client.cleanup();

	SDL_WaitThread(net, &netstatus);
	return 1;
}


int runServer()
{
	SDL_Thread* net = SDL_CreateThread(thread_net, "NetServerFF", (void*)RUN_TYPE::SERVER);
	int netstatus;

	std::ofstream log;
	log.open("Server_MainLog.txt");
	log << "Opening Server log...\n";
	GFX server = GFX();
	server.init("FreeFire Server", 1024, 768);
	while (running)
	{
		server.fill(255, 255, 255, 255);
		server.write("Hello, Server!", 10, 10);
		server.show();
		log << "Handling events...\n";
		handleEvents(running);
		SDL_Delay(16);
	}
	server.cleanup();

	SDL_WaitThread(net, &netstatus);
	return 1; 
}

int runModerator()
{
	SDL_Thread* net = SDL_CreateThread(thread_net, "NetModFF", (void*)RUN_TYPE::MODERATOR);
	int netstatus;

	std::ofstream log;
	log.open("Moderator_MainLog.txt");
	log << "Opening Server log...\n";
	GFX mod = GFX();
	mod.init("FreeFire Server", 1024, 768);
	while (running)
	{
		mod.fill(255, 255, 255, 255);
		mod.write("Hello, Moderator!", 10, 10);
		mod.show();
		log << "Handling events...\n";
		handleEvents(running);
		SDL_Delay(16);
	}
	mod.cleanup();

	SDL_WaitThread(net, &netstatus);
	return 1;
}

int runTeamBoard()
{
	SDL_Thread* net = SDL_CreateThread(thread_net, "NetBoardFF", (void*)RUN_TYPE::TEAM_BOARD);
	int netstatus;

	std::ofstream log;
	log.open("Teamboard_MainLog.txt");
	log << "Opening Team Board log...\n";
	GFX server = GFX();
	server.init("FreeFire Team Board", 1024, 768, true);
	while (running)
	{
		server.fill(255, 255, 255, 255);
		server.write("Hello, Team board!", 10, 10);
		server.show();
		log << "Handling events...\n";
		handleEvents(running);
		SDL_Delay(16);
	}
	server.cleanup();

	SDL_WaitThread(net, &netstatus);
	return 1;
}

int thread_net(void* netType)
{

	RUN_TYPE runtype = (RUN_TYPE)(unsigned int)netType;
	Uint32 PORT = 3991;
	std::string host = "127.0.0.1";
	std::ofstream log;
	/*=============================================================
	===============================================================
	Server
	===============================================================
	==============================================================*/
	if (runtype == SERVER)
	{
		log.open("Server_NetLog.txt");
		log << "Opening server log...\n";

		UDPsocket serversocket;
		serversocket = SDLNet_UDP_Open(PORT);
		UDPServer server = UDPServer(serversocket);
		while (running)
		{
			log << "Checking for packets";
			if (server.getPacket() > 0)
			{
				int signature = 0;
				memcpy(&signature, (char*)server.packet->data, sizeof(int));
				log << "Got packet with signature " << signature << std::endl;
			}
			SDL_Delay(1000);
		}
		SDLNet_UDP_Close(serversocket);
	}
	/*=============================================================
	===============================================================
	Client
	===============================================================
	==============================================================*/
	else if (runtype == CLIENT)
	{
		log.open("Client_NetLog.txt");
		log << "Opening client log...\n";
		UDPClient client = UDPClient(host, 3991, -1);
		client.init();
		while (running)
		{
			FFPacketPos generatedItem = RandomPacketPos();
			FFPacketBuild(client, &generatedItem);
			client.send();
			SDL_Delay(1000);
		}
	}
	/*=============================================================
	===============================================================
	Team Board
	===============================================================
	==============================================================*/
	else if (runtype == TEAM_BOARD)
	{
		log.open("Teamboard_NetLog.txt");
		log << "Opening teamboard log...\n";
	}

	if (log.is_open())
		log.close();
	return 1;
}

int thread_sim(void* data){return 0;}

/**

UDPsocket udps;
Uint32 SERVERPORT = 3991;

udps = SDLNet_UDP_Open(SERVERPORT);

if (type == CLIENT)
{
UDPClient client = UDPClient("2620:22:4000:31a:bdf5:993b:183b:d4c5", 3991, -1);
client.init();
int t=0;
while (running)
{
FFPacketPos generatedItem = RandomPacketPos();
FFPacketBuild(client, &generatedItem);
client.send();
}
}

if (type == SERVER)
{
UDPServer server = UDPServer(udps);
server.init();

while (running){
if (server.getPacket() > 0)
{
int signature=0;
memcpy(&signature,(char*)server.packet->data,sizeof(int));

if(signature == PTYPE_TIMESTAMP)
{
FFPacketTimestamp p;
FFPacketLoad<FFPacketTimestamp>(&p,server);
std::stringstream ss;
ss << "Timestamp: ";
ss << p.time;
}
if(signature == PTYPE_POS)
{
FFPacketPos p;
FFPacketLoad(&p,server);
}

}
SDL_Delay(100);
}
}
SDLNet_UDP_Close(udps);
*/