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
	void clear();

	template <typename Speaker, typename Listener>
	void translate(std::string message, Listener* listener)
	{
		FF::translate<Speaker, Listener>(message, listener);
	}
};

namespace FF{
	template<typename Speaker, typename Listener> struct translate {
		std::string operator()(std::string message, Listener* listener) {
			std::stringstream ss;
			ss << "Tool_Messenger.h can't translate : " << message;
			std::cout << ss.str();
			return ss.str();
		}
	};
}