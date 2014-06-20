#pragma once
#include <SDL2\SDL_net.h>

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
};

#define PBUILD(x) template<> void FFPacketBuild<x>(UDPClient& client, x* obj)
#define PLOAD(x) template<> void FFPacketLoad<x>(x* obj, UDPServer& server)
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
void FFPacketLoad(T* obj, UDPServer& server) {
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
			Set up a handshake with a remote peer
===============================================================
==============================================================*/

enum SHAKETYPE
{
	ANONYMOUS,
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

PBUILD(FFPShake) // client, obj
{
	client.put(obj->signature);
	client.put(obj->type);
}

PLOAD(FFPShake) // obj, server
{
	server.pos = 0;
	obj->signature = server.get<int>();
	obj->type = server.get<int>();
	server.pos = 0;
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