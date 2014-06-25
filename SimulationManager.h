#pragma once
#include <SDL2\SDL_mutex.h>
#include <SDL2\SDL_image.h>
#include <unordered_map>
#include <map>
#include "pos.h"

// TODO : Template the class so that the ints are typename T
class SimulationManager
{
public:
	typedef std::unordered_map<const pos, int, poshash, poseq> t_chunk;
	typedef std::unordered_map<std::string, t_chunk> t_grid;
	//have a map that holds
	//a map of spacially hashed
	//chunks that hold a set of data items.
	// simulation.get("trees",pos(1,5),pos(2,2)); "trees" -> cell(1,5) -> cell(2,2)
	// simulation.get("trees", pos(2,17));
	std::unordered_map<
		std::string,
		std::unordered_map<const pos, int, poshash, poseq>
		> grid;
	//since the network sends tiles as IDs, they'll
	//have to sync the asset id's with the file name
	//that loads them.
	std::unordered_map<int, std::string> lookup;

	bool hasLookup(const int& key);
	void updateTile(const std::string& group, const pos& cell, int newIndex);
	int getTile(const std::string& group, const pos& cell);
	void createLookup(const int& key, const std::string& path);
	std::string path(const int& key);
	SimulationManager();
	~SimulationManager();
};

/*
Layers
chunks
tiles
*/