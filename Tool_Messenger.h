#pragma once
#include "Tool.h"
#include <iosfwd>

/*

The messenger should be able to build and send packets through the network.
It should then be able to read and translate them.

build buffer from string
send buffer through network
receive buffer from network

Should be configured to a certain messenger type

*/
class Tool_Messenger :
	public Tool
{
private:
	class Impl
	{
	public:
		Impl();
		void write(const std::string& data, Uint8* start);
		std::string read(Uint8* start);
	};
	std::unique_ptr<Impl> p;

	Tool_Messenger(const Tool_Messenger&) = delete;

public:
	Tool_Messenger();
	~Tool_Messenger();
	void write(const std::string& data, Uint8* start);
	std::string read(Uint8* start);
};