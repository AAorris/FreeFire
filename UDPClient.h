#pragma once

#include <iostream>
#include <string>
#include <SDL2\SDL_net.h>

class UDPClient {
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

	UDPClient();
	UDPClient(std::string p_host, int p_port, int p_channel=-1);
	~UDPClient();
	void zero();

	void init();
	void send();
	void clear();
	
	void putString(std::string s);

	template <typename T>
	void put(const T& item)
	{
		memcpy(&packet->data[pos],&item,sizeof(item));
		pos+=sizeof(item);
	}

	template <typename T>
	T get()
	{
		T result = *(&packet->data[pos]);
		pos += sizeof(T);
		return result;
	}

	template<> std::string get<std::string>()
	{
		std::string result = std::string((char*)&(packet->data[pos]));
		pos += sizeof(char)*result.size();
	}

};