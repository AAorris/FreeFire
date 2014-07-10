#pragma once
#include "Tool.h"
#include <iosfwd>
class Tool_Asset :
	public Tool
{
private:
	static std::vector<std::string> lookup;
	class Impl;
	std::unique_ptr<Impl> p;
public:

	int id;
	//std::string path;

	Tool_Asset(const std::string& s, void* ren, int pid=-1);
	virtual ~Tool_Asset();
	Tool_Asset(const Tool_Asset& c) = delete;
	Tool_Asset(Tool_Asset&& other);
	bool operator==(const Tool_Asset& a) const;
	void draw(int x, int y) const;
	std::string getPath();
	static int useLookup(const std::string& s);

}; 

typedef Tool_Asset _asset;

namespace std {
	template<> struct hash<_asset>
	{
		size_t operator()(const _asset& asset) 
		{ return asset.id; }
	};
}

template< typename T >
using sdl_uptr = unique_ptr< T, SDL::default_delete<T> >;

namespace SDL
{
	template<> struct default_delete<SDL_Surface> {
		void operator()(SDL_Surface* s){ SDL_FreeSurface(s); }
	};
	template<> struct default_delete<SDL_Texture> {
		void operator()(SDL_Texture* s){ SDL_DestroyTexture(s); }
	};
}