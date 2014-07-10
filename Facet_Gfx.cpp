#include "stdafx.h"
#include "Facet_Gfx.h"
#include <algorithm>
#include <unordered_map>

#define CLASS Facet_Gfx
#include "PIMPL.h"

struct IMPL {
	typedef std::unordered_set< std::unique_ptr<_asset> > t_container;
	typedef const std::unique_ptr<_asset>& i_container;

	std::unordered_map<std::string, sdl_uptr<SDL_Texture>> cached_chunks;

	sdl_uptr<SDL_Window> window;
	sdl_uptr<SDL_Renderer> renderer;

	t_container assets;

	Impl() :
		window{ SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_RENDERER_TARGETTEXTURE) },
		renderer { SDL_CreateRenderer(window.get(),-1,SDL_RENDERER_ACCELERATED) }
	{
	}

	t_container::iterator find(const int& n)
	{
		return std::find_if(
			begin(assets), end(assets), 
				[n](i_container a)
				{
					return a->id == n;
				}
		);
	}

	t_container::iterator find(const std::string& s)
	{
		return std::find_if(
			begin(assets), end(assets), 
				[s](i_container it) {
					return it->getPath() == s;	
				}
			);
	}

	void draw(int asset, int x, int y)
	{
		draw(find(asset), x, y);
	}

	void draw(t_container::iterator it, int x = 0, int y=0)
	{
		if (it == end(assets))
			return;
		(*it)->draw(x,y);
	}

	void loadAsset(const std::string& s)
	{
		//s is signed with a unique index
		char id = s[0] - '0';
		std::string path = s.substr(1, s.size());
		assets.insert(wrap( new _asset(path, renderer.get(), id) ));
	}

	void cache(const std::string& chunkData)
	{
		auto tex = sdl_uptr<SDL_Texture>
		{ 
			SDL_CreateTexture(
				renderer.get(), 
				SDL_PIXELFORMAT_RGBA8888, 
				SDL_TEXTUREACCESS_TARGET, 
				CHUNK::width*GFX::res, 
				CHUNK::height*GFX::res
			)
		};

		SDL_SetRenderTarget(renderer.get(), tex.get());
		for (int x = 0; x < CHUNK::width; x++)
		{
			for (int y = 0; y < CHUNK::height; y++)
			{
				int loc = x + CHUNK::width*y;
				char asset = chunkData[loc];
				if (asset > 0)
				{
					draw(asset, x, y);
				}
			}
		}
		SDL_SetRenderTarget(renderer.get(), NULL);

		cached_chunks.insert(std::make_pair(chunkData,std::move(tex)));
	}

	void drawChunk(const std::string& chunkData, const AA::Pos& pos)
	{

	}
};

//template <typename T>
void CLASS::draw(const int& key)
{
	if (key == 0)
		return;
	auto location = p->find(key);
	if (location == end(p->assets))
		return;
	p->draw(location);
}

//template <typename T>
void CLASS::draw(const std::string& key)
{
	auto location = p->find(key);
	if (location == end(p->assets))
		return;
	p->draw(location);
}

void CLASS::drawChunk(unique_ptr<_sim>& sim, const AA::Pos& pos)
{
	std::string chunkData = sim->getChunk(pos);
	if (chunkData.size() == 16)
	{
		p->cache(chunkData);
		p->drawChunk(chunkData, pos);
	}
}

/*--------------------------------------------------------*/

CTOR() : p{ new Impl() } {}
DTOR() {}

void CLASS::loadAsset(const std::string& path)
{
	p->loadAsset(path);
}

void CLASS::present()
{
	SDL_RenderPresent(p->renderer.get());
}