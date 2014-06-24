#pragma once
#ifndef PACKETS
#define PACKETS
#include <SDL2\SDL_net.h>
#include <string>


#define PTYPE_TIMESTAMP 1
#define PTYPE_MESSAGE 2
#define PTYPE_POS 3
#define PTYPE_CLIENTSHAKE 4

enum PTYPE
{
	PNONE,
	PTIME,
	PMESG,
	PPOS,
	PSHAKE,
	PASSET,
	PTILE,
};

#define PBUILD(x) template<> void FFPacketBuild<x>(UDPClient& client, x* obj)
#define PLOAD(x) template<> void FFPacketLoad<x,M>(x* obj, M& manager)
#define PCREATE(x) template<> x FFPacketCreate<x>()

/*=============================================================
===============================================================
						Templates
===============================================================
==============================================================*/

template <typename T>
void FFPacketBuild(UDPClient& client, T* obj){
	//specialize me to build client's packet from object
	throw 0;
}
template <typename T>
void FFPacketBuildServer(UDPServer& server, T* obj){
	//specialize me to build server's packet from object
	throw 0;
}
template <typename T, typename M>
void FFPacketLoad(T* obj, M& manager) {
	//specialize me to read server's packet into object
	throw 0;
}
template <typename T>
T FFPacketCreate()
{
	//specialize me to create an object from scratch (initialization only)
	throw 0;
}

/*=============================================================
===============================================================
			Hold a table for referencing assets
===============================================================
==============================================================*/

enum ASSETSTATUS {
	STATUS_NONE,
	STATUS_LOADING,
	STATUS_LOADED
};

struct FFPAsset
{
	int signature;
	ASSETSTATUS status;
	std::string path;
};

FFPAsset makeAssetRequest(std::string assetPath)
{
	FFPAsset req;
	req.signature = PTYPE::PASSET;
	req.status = ASSETSTATUS::STATUS_NONE;
	req.path = assetPath;
	return req;
}

template<> void FFPacketBuildServer<FFPAsset>(UDPServer& server, FFPAsset* obj)
{
	server.pos = 0;
	server.put<int>(obj->signature);
	server.put<int>(obj->status);
	server.putString(obj->path);
	server.pos = 0;
}


void sendAssetRequest(const std::string& assetPath, UDPServer& server, const IPaddress& dest)
{
	FFPAsset req = makeAssetRequest(assetPath);
	FFPacketBuildServer<FFPAsset>(server, &req);
	//server.packet->address = dest;
	server.send(dest);
}

/*=============================================================
===============================================================
			Send an update to a tile on the map
===============================================================
==============================================================*/

struct FFPTileUpdate
{
	int signature;
	int x;
	int y;
	int tileID;
};

template <typename M = UDPServer>
void sendTileUpdate(const int& x, const int& y, const int& assetID, M& manager, const IPaddress& dest)
{
	manager.put<int>(PTILE);
	manager.put<int>(x);
	manager.put<int>(y);
	manager.put<int>(assetID);
	manager.send(dest);
}

template <typename M = UDPClient>
FFPTileUpdate getTileUpdate(M& manager)
{
	FFPTileUpdate data;
	data.signature = manager.get<int>();
	data.x = manager.get<int>();
	data.y = manager.get<int>();
	data.tileID = manager.get<int>();
	manager.clear();
	return data;
}

/*=============================================================
===============================================================
			Set up a handshake with a remote peer
===============================================================
==============================================================*/

enum SHAKETYPE
{
	ANONYMOUS,
	ACCEPT,
	DENY
};

struct FFPShake
{
	int signature;
	int type;
};

PCREATE(FFPShake)
{
	FFPShake obj = FFPShake();
	obj.signature = PTYPE::PSHAKE;
	obj.type = SHAKETYPE::ANONYMOUS;
	return obj;
}
template<> void FFPacketBuildServer<FFPShake>(UDPServer& server, FFPShake* obj)
{
	server.clear();
	server.put(obj->signature);
	server.put(obj->type);
}
PBUILD(FFPShake) // client, obj
{
	client.clear();
	client.put(obj->signature);
	client.put(obj->type);
}

template <typename M>
void PLoadShake(FFPShake* obj, M& manager)
{
	manager.pos = 0;
	obj->signature = manager.get<int>();
	obj->type = manager.get<int>();
	manager.pos = 0;
}

/*=============================================================
===============================================================
				Simple Example(time stamp)
===============================================================
==============================================================*/
 
struct FFPacketTimestamp
{
	int signature;
	int time;
};

template<> void FFPacketBuild<FFPacketTimestamp>(UDPClient& client, FFPacketTimestamp* obj)
{
	client.put(obj->signature);
	client.put(obj->time);
}

/**Load p into obj */
template <> void FFPacketLoad<FFPacketTimestamp>(FFPacketTimestamp* obj, UDPServer& server)
{
	server.pos=0;
	obj->signature = server.get<int>();
	obj->time = server.get<int>();
	server.pos=0;
}

/*=============================================================
===============================================================
					Test - Position
===============================================================
==============================================================*/

struct FFPacketPos
{
	int signature;
	int type;
	int id;
	int x;
	int y;
};

FFPacketPos RandomPacketPos()
{
	FFPacketPos pos;
	pos.signature = PTYPE_POS;
	pos.type = 1;
	pos.x = rand()%100;
	pos.y = rand()%100;
	return pos;
}

template<> void FFPacketBuild<FFPacketPos>(UDPClient& client, FFPacketPos* obj)
{
	client.put(obj->signature);
	client.put(obj->type);
	client.put(obj->id);
	client.put(obj->x);
	client.put(obj->y);
}

/**Load p into obj */
template <> void FFPacketLoad<FFPacketPos>(FFPacketPos* obj, UDPServer& server)
{
	server.pos=0;
	obj->signature = server.get<int>();
	obj->type = server.get<int>();
	obj->id = server.get<int>();
	obj->x = server.get<int>();
	obj->y = server.get<int>();
	server.pos=0;
}

#endif