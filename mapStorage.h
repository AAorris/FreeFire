#pragma once

#include <iostream>
#include <SDL2/SDL.h>

/**
mapStorage is a template class for interfacing with the underlying map algorithm.
However it's managed, the program needs to be able to put data in, and get it out,
and reference the assets used to render it.
*/
template <typename T, typename A=SDL_Texture*, typename K=std::string>
class mapStorage
{
	T* storage;
	mapStorage(T* t)
	{
		storage = t;
	}
	void put(const A& asset)
	{
		storage.put(asset);
	}
	const K& get(const K& key)
	{
		return storage.get(key);
	}
};