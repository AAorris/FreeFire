#pragma once
#include "Module.h"
#include "Tool_Pos.h"
#include <iosfwd>
#include "Facet_Sim.h"
#include "Tool_Messenger.h"

class Module_Fire :
	public Module
{
private:
	class Impl;
	unique_ptr<Impl> p;
public:
	Module_Fire();
	Module_Fire(const Module_Fire& f);
	~Module_Fire();

	void startFire(const AA::Pos& p);
	void update(const int& step);
	std::set<std::string> getNews();
	void loadState(const std::string& s);
};

namespace FF{
	/*template<typename Speaker, typename Listener> struct translate {
		void operator()(std::string message, Listener* listener) const {
			std::cout << "Tool_Messenger.h can't translate : " << message;
		}
	};*/
	template <> struct translate<Module_Fire, Facet_Sim> {
		std::string operator()(std::string message, t_sim* listener) {
			std::stringstream ss;
			if (message.substr(0, 1) == "F") {
				ss << message;
				std::string type;
				short px, py;
				ss >> type >> px >> py;
				listener->put(AA::Pos(px,py), 'F');
			}
			else {
				ss << "" << message << "\n";
				std::cout << ss.str();
			}
			return ss.str();
		}
	};
}

typedef Module_Fire Fire;