#include <SDL2\SDL_main.h>
#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <queue>

#include <SDL2\SDL.h>
#include <SDL2\SDL_image.h>
#include <SDL2\SDL_ttf.h>
#include <SDL2\SDL_net.h>

#include "pos.h"

#include "UDPClient.h"
#include "UDPServer.h"
#include "SimulationManager.h"

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

SDL_mutex* mutex_sim;
SimulationManager sim;
std::queue<FFPTileUpdate> tileQueue;
std::queue<std::string> newAssets;

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
	menu1.init("Free Fire Menu", 600, 300, true);
	mutex_sim = SDL_CreateMutex();
	tileQueue = std::queue<FFPTileUpdate>();

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
	GFX gfx = GFX();
	gfx.init("FreeFire Client", 300, 300);
	while (running)
	{
		gfx.fill(255, 255, 255, 255);
		gfx.write("Hello, Client!", 10, 10);

		if (newAssets.empty() == false)
		{
			if (SDL_TryLockMutex(mutex_sim) == 0) {
				while (newAssets.empty() == false) {
					
					std::string path = newAssets.front();
					log << "Loading path: " << path << "...";
					if (gfx.loadAsset(newAssets.front()))
					{
						log << " success.\n";
					}
					else
						log << " failure! " << IMG_GetError() << "\n";
					
					
					newAssets.pop();
				}
				SDL_UnlockMutex(mutex_sim);
			}
		}

		for (int x = 0; x < 10; x++)
		{
			for (int y = 0; y < 10; y++)
			{
				int value = sim.getTile("map", pos(x, y));
				if (value != -1)
				{
					SDL_Rect rect = { x * 10, y * 10, 10, 10 };
					if (sim.hasLookup(value))
					{
						std::string key = sim.path(value);
						if (key != "")
						{
							if (gfx.textures.find(key) != gfx.textures.end())
							{
								SDL_RenderCopy(gfx.ren, gfx.textures.at(key), NULL, &rect);
							}
							else
							{
								SDL_SetRenderDrawColor(gfx.ren, 255, 0, 0, 255);
								SDL_RenderDrawRect(gfx.ren, &rect);
								newAssets.push(key);
							}
						}
					}
				}
			}
		}

		gfx.show();
		log << "Handling events...\n";
		handleEvents(running);
		SDL_Delay(16);
	}
	gfx.cleanup();

	SDL_WaitThread(net, &netstatus);
	return 1;
}


int runServer()
{

	std::ofstream log;
	log.open("Server_MainLog.txt");
	log << "Opening Server log...\n";
	GFX gfx = GFX();
	gfx.init("FreeFire Server", 300, 300);

	SimulationManager serverSim = SimulationManager();

	//load some hard coded assets for testing purposes
	gfx.loadAsset("tree.png");
	sim.lookup.insert(std::make_pair(0, "tree.png"));
	gfx.loadAsset("pineTree.png");
	sim.lookup.insert(std::make_pair(1, "pineTree.png")); 
	gfx.loadAsset("fire_breaker.png");
	sim.lookup.insert(std::make_pair(2, "fire_breaker.png"));
	gfx.loadAsset("fire_fighter.png");
	sim.lookup.insert(std::make_pair(3, "fire_fighter.png"));
	gfx.loadAsset("fire.png");
	sim.lookup.insert(std::make_pair(4, "fire.png"));


	SDL_Thread* net = SDL_CreateThread(thread_net, "NetServerFF", (void*)RUN_TYPE::SERVER);
	int netstatus;

	while (running)
	{
		gfx.fill(255, 255, 255, 255);
		gfx.write("Hello, Server!", 10, 10);
		gfx.show();

		if (SDL_TryLockMutex(mutex_sim) == 0) {
			//log << "In mutex\n";
			SDL_UnlockMutex(mutex_sim);
		}
		//log << "Handling events...\n";
		handleEvents(running);
		SDL_Delay(16);
	}
	gfx.cleanup();

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

		//UDPsocket serversocket;
		//serversocket = SDLNet_UDP_Open(PORT);
		UDPServer server = UDPServer(host,PORT,-1);
		server.init();

		while (running)
		{
			//try locking the simulation mutex for laughs

			//poor simulation module.
			//SDL_TryLockMutex(mutex_sim);

			/*
			CHECK FOR INCOMING PACKETS
			*/
			if (server.getPacket() > 0)
			{
				int signature = 0;
				memcpy(&signature, (char*)server.packet->data, sizeof(int));
				log << "Got packet with signature " << signature << std::endl;
				/*
				Handle incoming handshakes from new clients
				*/
				if (signature == PASSET)
				{
					FFPAssetMessage msg = getAssetResponse(server);
					if (msg.context == ASSET_REQUEST)
					{
						if (sim.lookup.count(msg.assetID) != 0)
						{
							log << "Some client is asking for an asset they don't recognize, I guess I'll help them out.\n";
							sendAssetResponse(msg, sim.lookup.at(msg.assetID), server, server.packet->address);
						}
						else
						{
							log << "Client is asking for an asset I don't have!(" << msg.assetID << ")\n";
						}
					}
				}
				if (signature == PTYPE::PSHAKE) {
					FFPShake incomingShake;
					PLoadShake<UDPServer>(&incomingShake, server);
					//PacketBuildServer(server, &incomingShake);

					log << "...A hand shake, with type : " << incomingShake.type << "\n";

					if (incomingShake.type == ANONYMOUS)
					{
						//Some unknown client is trying to shake hands
						//Send an acceptance of course; you're a nice guy after all
						//	- after you start shaking their hand, you wait for them
						//	- to stop shaking your hand...
						//	- you can stop now...
						if (server.clients.find(server.packet->address) == server.clients.end()) {
							if (server.shaking.find(server.packet->address) == server.shaking.end())
							{
								log << "Oh look! Someone is trying to shake my hand. I'll accept.\n";
								server.shaking.insert(server.packet->address);
							}
							else // this address is already on the shaking list, but not the client list.
							{
								log << "Uh, you can stop shaking my hand now...\n";
							}
							FFPShake returnShake = FFPacketCreate<FFPShake>();
							returnShake.type = SHAKETYPE::ACCEPT;
							FFPacketBuildServer<FFPShake>(server, &returnShake);
							server.send(server.packet->address);
						}
					}

					else if (incomingShake.type == SHAKETYPE::ACCEPT)
					{
						log << "This hand shake is an acceptance!\n";
						//client acknowledged our shake response
						//client is done shaking our hand
						//finally add client to client list
						if (server.shaking.find(server.packet->address) != server.shaking.end())
						{
							log << "Done shaking " << server.packet->address.host << ":" << server.packet->address.port << "'s hand. Now we've met.\n";
							server.clients.insert(server.packet->address);
							server.shaking.erase(server.packet->address);
						}
					}
				}
			}
			if (server.clients.size() > 0)
			{
				/*
				Send updates to relevant existing clients
				*/
				for (auto i = server.clients.begin(); i != server.clients.end(); i++)
				{
					int treeTile = rand()%4;
					sendTileUpdate("map", rand()%10, rand()%10, treeTile,server,*i);
				}
			}
			SDL_Delay(100);
		}
		SDLNet_UDP_Close(server.socket);
		//SDLNet_UDP_Close(serversocket);
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

		bool shook = false;

		while (running)
		{
			//get the server to say hi!
			if (!shook) {
				FFPShake outgoingShake = FFPacketCreate<FFPShake>();
				FFPacketBuild(client, &outgoingShake);
				client.send();
				log << "Please shake my hand Mr. Server!\n";
			}
			//check if the server has sent us any information
			while (SDLNet_UDP_Recv(client.socket, client.packet))
			{
				int signature = 0;
				memcpy(&signature, (char*)client.packet->data, sizeof(int));
				log << "Got packet with signature " << signature << std::endl;

				if (signature == PASSET)
				{
					FFPAssetMessage msg = getAssetResponse(client);
					if (msg.context == ASSETCONTEXT::ASSET_REPLY) {
						log << "Received asset reply : (" << msg.assetID << "," << msg.assetName << ")\n";
						sim.lookup.insert(std::make_pair(msg.assetID, msg.assetName));
					}
				}
				if (signature == PTILE)
				{
					log << "\tIt's a tile update!\n";
					FFPTileUpdate newTile = getTileUpdate(client);
					if (SDL_TryLockMutex(mutex_sim)==0)
					{
						log << "\tnew tile : (" << newTile.x << ", " << newTile.y << ", " << newTile.tileID << ")\n";
						if (sim.hasLookup(newTile.tileID) == false)
						{
							log << "I don't know the image for this tile id. Asking Mr. Server!\n";
							sendAssetRequest(newTile.tileID, client, client.ipserver);
						}
						sim.updateTile(newTile.group,pos(newTile.x,newTile.y),newTile.tileID);
						while (tileQueue.empty() == false)
						{
							newTile = tileQueue.front();
							sim.updateTile(newTile.group, pos(newTile.x, newTile.y), newTile.tileID);
							if (sim.hasLookup(newTile.tileID) == false)
							{
								sendAssetRequest(newTile.tileID, client, client.ipserver);
							}
							tileQueue.pop();
						}
						SDL_UnlockMutex(mutex_sim);
					}
					else {
						tileQueue.push(newTile);
						log << "Couldn't lock the mutex! Adding update to queue.\n";
					}
				}
				if (!shook && signature == PSHAKE)
				{
					FFPShake incomingShake;
					PLoadShake(&incomingShake, client);

					if (incomingShake.type == SHAKETYPE::ACCEPT)
					{
						shook = true;
						log << "Mr. Server shook my hand! Stopping shaking.\n";
						FFPShake outgoingShake = FFPacketCreate<FFPShake>();
						outgoingShake.type = SHAKETYPE::ACCEPT;
						FFPacketBuild<FFPShake>(client, &outgoingShake);
						client.send();
					}
				}
			}
			//FFPacketPos pos = RandomPacketPos();
			//FFPacketBuild(client, &pos);
			//client.send();
			SDL_Delay(100);
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
