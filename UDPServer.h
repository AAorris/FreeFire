#pragma once

#include <iostream>
#include <string>
#include <SDL2\SDL_net.h>

class UDPServer {
public:
	std::string hostname;
	int port;
	int packetSize;
	int channel;

	//current position in the data
	int pos;

	IPaddress ipserver;
	//IPaddress ipclient;
	UDPpacket * packet;
	UDPsocket socket;

	UDPServer();
	UDPServer(std::string p_host, int p_port, int p_channel=-1);
	UDPServer(UDPsocket s);
	~UDPServer();
	void zero();

	void init();
	void clear();
	int getPacket();

	//void getString(std::string s);

	template <typename T>
	T get()
	{
		T result = *(&packet->data[pos]);
		pos+=sizeof(T);
		return result;
	}

	template<> std::string get<std::string>()
	{
		std::string result = std::string((char*)&(packet->data[pos]));
		pos+=sizeof(char)*result.size();
	}
};