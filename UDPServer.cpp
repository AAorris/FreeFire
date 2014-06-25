#include "UDPServer.h"

/**Clean up variable garbage*/
UDPServer::UDPServer()
{
	zero();
}

UDPServer::UDPServer(std::string p_host, int p_port, int p_channel)
{
	zero();
	hostname = p_host;
	port = p_port;
	channel = p_channel;
	socket = NULL;
}

UDPServer::UDPServer(UDPsocket s)
{
	zero();
	socket = s;
	hostname = "127.0.0.1";
	port = 3991;
	channel = -1;
}

/**Cleanup*/
UDPServer::~UDPServer()
{
	//ipaddress
	//SDLNet_FreePacket(packet); //packet
	//SDLNet_UDP_Close(socket); //socket
}

void UDPServer::zero()
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
void UDPServer::init()
{
	packet = SDLNet_AllocPacket(packetSize);
	clear(); //the packet
	if(socket==NULL)
		socket = SDLNet_UDP_Open(port);
	SDLNet_ResolveHost(&ipserver, NULL, port);
	packet->address.host = ipserver.host;
	packet->address.port = ipserver.port;
}

/**Fire the packet*/
void UDPServer::send(const IPaddress& ip)
{
	packet->len = pos + 1;
	packet->address = ip;
	SDLNet_UDP_Send(socket, channel, packet);
	clear();
}

int UDPServer::getPacket()
{
	return SDLNet_UDP_Recv(socket, packet);
}

/**Clear the data in the packet*/
void UDPServer::clear()
{
	SDLNet_FreePacket(packet);
	packet = SDLNet_AllocPacket(packetSize);
	//memset(packet->data,0,packetSize);
	packet->channel = channel;
	packet->len = 0;
	packet->status = 0;
	pos=0;
}

/**write a string into the packet*/
void UDPServer::putString(std::string s)
{
	const char* str = s.c_str();
	char* dest = (char*)calloc(sizeof(char), strlen(str)+1);
	//strcpy_s(dest, strlen(str)+1, str);
	//could this be 4 bytes off?
	strcpy_s((char*)(packet->data + pos), strlen(str)+1, str);
	pos += s.length();
}