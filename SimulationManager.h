#pragma once
#include <SDL2\SDL_mutex.h>
#include <unordered_map>
#include "pos.h"

class SimulationManager
{
public:
	std::unordered_multimap<const pos, int, poshash, poseq> tiles;
	std::unordered_map<int, std::string> lookup;

	bool hasLookup(const int& key);
	void updateTile(const pos& p, int newIndex);
	SimulationManager();
	~SimulationManager();
};

