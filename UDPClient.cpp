#include "UDPClient.h"

/**Clean up variable garbage*/
UDPClient::UDPClient()
{
	zero();
}

UDPClient::UDPClient(std::string p_host, int p_port, int p_channel)
{
	zero();
	hostname = p_host;
	port = p_port;
	channel = p_channel;
}

/**Cleanup*/
UDPClient::~UDPClient()
{
	//ipaddress
	//SDLNet_FreePacket(packet); //packet
	//SDLNet_UDP_Close(socket); //socket
}

void UDPClient::zero()
{
	packetSize = 512;
	packet = 0;
	channel = -1;
	pos = 0;
	hostname = "127.0.0.1";
}
/**
	Initialize the client :
	Allocate a packet structure
	Open a free port to send from
	Resolve the server's address
	Point the packet at the server
*/
void UDPClient::init()
{
	packet = SDLNet_AllocPacket(packetSize);
	clear(); //the packet
	socket = SDLNet_UDP_Open(0);
	SDLNet_ResolveHost(&ipserver, hostname.c_str(), port);
	packet->address.host = ipserver.host;
	packet->address.port = ipserver.port;
}

/**Fire the packet*/
void UDPClient::send()
{
	packet->len = pos+1;
	SDLNet_UDP_Send(socket, channel, packet);
	clear();
}

/**Clear the data in the packet*/
void UDPClient::clear()
{
	memset(packet->data,0,packetSize);
	packet->channel = channel;
	packet->len = 0;
	packet->status = 0;
	pos=0;
}

/**write a string into the packet*/
void UDPClient::putString(std::string s)
{
	strcpy_s((char*)(packet->data + pos),s.size(), s.c_str());
	pos+=s.length();
}