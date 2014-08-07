#pragma once

#include <boost\property_tree\ptree.hpp>
#include <SDL2\SDL.h>

struct menuItem {
	boost::property_tree::ptree data;
	SDL_Texture* texture;
	SDL_Rect rect;
	//scalar origin;
	//scalar size;
};

class Menu
{
public:
	std::vector<menuItem> menuItems;


	Menu();
	~Menu();
};

