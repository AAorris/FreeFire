#pragma once

#include <iostream>
#include <string>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <SDL2\SDL_net.h>

struct IPHash {
	size_t operator() (const IPaddress &addr) const {
		std::size_t host = std::hash<unsigned int>()((unsigned int)addr.host);
		std::size_t port = std::hash<unsigned int>()((unsigned int)addr.port);
		return host ^ (port << 1);
	}
};

struct IPEq {
	bool operator() (IPaddress const& t1, IPaddress const& t2) const
	{
		return (t1.host == t2.host) && (t1.port == t2.port);
	}
};

class UDPServer {
public:
	std::unordered_set<IPaddress,IPHash,IPEq> shaking;
	std::unordered_set<IPaddress,IPHash,IPEq> clients;
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
	void send(const IPaddress& ip);
	void clear();
	int getPacket();

	//void getString(std::string s);
	void putString(std::string s);

	template <typename T>
	T get()
	{
		T result = *(&packet->data[pos]);
		pos+=sizeof(T);
		return result;
	}

	template<> std::string get<std::string>()
	{
		std::string result = (char*)(&packet->data[pos]);
		pos += sizeof(char)*result.size();
		return result;
	}


	template <typename T>
	void put(const T& item)
	{
		memcpy(&packet->data[pos], &item, sizeof(item));
		pos += sizeof(item);
	}

};