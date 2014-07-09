#include "stdafx.h"
#include "Facet_Gfx.h"
#include <algorithm>

#define CLASS Facet_Gfx
#include "PIMPL.h"

struct IMPL {
	typedef std::unordered_set<_asset> t_container;
	typedef const _asset& i_container;

	sdl_uptr<SDL_Window> window;
	sdl_uptr<SDL_Renderer> renderer;

	t_container assets;

	Impl() :
		window{ SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, 0) },
		renderer { SDL_CreateRenderer(window.get(),-1,SDL_RENDERER_ACCELERATED) }
	{
	}

	t_container::iterator find(const int& n)
	{
		return std::find_if(
			begin(assets), end(assets), 
				[n](i_container a)
				{
					return a.id == n; 
				}
		);
	}

	t_container::iterator find(const std::string& s)
	{
		return std::find_if(
			begin(assets), end(assets),
				[s](i_container a)
				{
					return a.path == s;
				}
		);
	}

	void draw(t_container::iterator it)
	{
		
	}

	void loadAsset(const std::string& s)
	{
		assets.insert(_asset(s, renderer.get()));
	}
};

/*--------------------------------------------------------*/

CTOR() : p{ new Impl() } {}
DTOR() {}

void CLASS::loadAsset(const std::string& path)
{
	p->loadAsset(path);
}