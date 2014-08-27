#pragma once
#include "Tool.h"
#include <iosfwd>
class Tool_Asset :
	public Tool
{
private:
	//PIMPL
	class Impl;
	std::unique_ptr<Impl> p;
public:

	Tool_Asset(const std::string& s, void * ren );
	virtual ~Tool_Asset();
	Tool_Asset(const Tool_Asset& c) = delete;
	Tool_Asset(Tool_Asset&& other);
	static Tool_Asset make_item(const std::string& label, const std::string& path);
	bool operator==(const Tool_Asset& a) const;
	bool operator<(const Tool_Asset& a) const;
	void draw(int x, int y, int w, int h, bool centered) const;
	void draw(SDL_Rect* rect) const;
	std::string getPath() const;
	SDL_Texture* getTexture() const;
	int getSize() const;
}; 

typedef Tool_Asset _asset;

namespace std {
	template<> struct hash<_asset>
	{
		size_t operator()(const _asset& asset) 
		{ return std::hash<std::string>()(asset.getPath()); }
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