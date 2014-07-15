#include "stdafx.h"
#include "Facet_Gfx.h"
#include <algorithm>
#include <unordered_map>

#define CLASS Facet_Gfx
#include "PIMPL.h"

struct IMPL {
	typedef std::unordered_set< _asset >	container_type;
	typedef container_type::iterator		container_iterator;
	typedef const _asset&					container_value;

	std::unordered_map<AA::Pos, sdl_uptr<SDL_Texture>> cached_chunks;

	SDL_Window* window;
	SDL_Renderer* renderer;

	container_type assets;

	int res;
	double offsetX;
	double offsetY;
	double originX;
	double originY;

	Impl()
	{
		SDL_DisplayMode screen;
		SDL_GetCurrentDisplayMode(0, &screen);

		screen.w *= 0.8;
		screen.h *= 0.8;

		res = 32;
		offsetX = 0;
		offsetY = 0;
		originX = screen.w / 2;
		originY = screen.h / 2;

		window = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen.w, screen.h, 0);
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	}

	Impl(Impl&& i)
	{
		window = i.window;
		renderer = i.renderer;
		i.window = NULL;
		i.renderer = NULL;
	}

	Impl& operator=(const Impl& i) = delete;

	~Impl() 
	{
		if (window!=0)
			SDL_DestroyWindow(window);
		if (renderer!=0)
			SDL_DestroyRenderer(renderer);
	}

	container_iterator find(const int& n)
	{
		auto test = [n](container_value i) { return i.id == n; };
		return std::find_if(assets.begin(), assets.end(), test);
	}

	container_iterator find(const std::string& s)
	{
		auto test = [s](container_value i) { return i.getPath() == s; };
		return std::find_if(assets.begin(), assets.end(), test);
	}

	void draw(SDL_Texture* tex, SDL_Rect r, bool isCellPos=false)
	{
		int screenW, screenH;
		screenW = screenH = 0;

		SDL_GetWindowSize(window, &screenW, &screenH);

		r.y *= -1;

		r.x += screenW / 2 - r.w/2;
		r.y += screenH / 2 - r.h/2;

		SDL_RenderCopy(renderer, tex, NULL, &r);
		SDL_RenderDrawRect(renderer, &r);
	}

	void draw(int asset, int x, int y)
	{
		draw(find(asset), x, y); 
	}

	void draw(container_iterator it, int x = 0, int y=0)
	{
		if (it == end(assets))
			return;
		it->draw(x,y);
	}

	void drawCell(container_iterator it, int x = 0, int y = 0, int w=-1, int h=-1)
	{
		if (it == end(assets))
			return;
		it->draw(x, y, w, h);
	}

	void loadAsset(const std::string& s, int id)
	{
		assets.insert(_asset(s, renderer, id));
	}
	void drawChunk(const std::string& chunkData, const AA::Pos& pos)
	{
		SDL_Texture* cachedChunk = NULL;

		SDL_SetRenderDrawColor(renderer, 60, 60, 60, 0);
		if (cached_chunks.count(pos) == 0)
		{
			//create a texture you can draw to
			cachedChunk =
				SDL_CreateTexture(
					renderer,
					SDL_PIXELFORMAT_RGBA8888,
					SDL_TEXTUREACCESS_TARGET,
					CHUNK::width*res, CHUNK::height*res
				)
			;
			if (cachedChunk==0)
				SDL_ShowSimpleMessageBox(0, "error", SDL_GetError(), window);

			using ptr = sdl_uptr<SDL_Texture>; //
			auto key = pos;
			cached_chunks.insert(
				std::make_pair(
					pos,
					ptr{ std::move(cachedChunk) }
				)
			);
		}
		else {
			cachedChunk = cached_chunks.at(pos).get();
		}

		if (SDL_SetRenderTarget(renderer, cachedChunk) == 0)
		{
			SDL_RenderClear(renderer);
			auto cell = AA::Pos{ 0, 0 };
			for (const char& c : chunkData) {
				//
				int _offset = res;
				int _max = res*CHUNK::width;

				drawCell(find((int)c), cell.x(), cell.y());

				cell.dx(_offset);
				if (cell.x() >= _max) {
					cell.x(0);
					cell.dy(_offset);
				}
				//
			}
			SDL_SetRenderTarget(renderer, NULL);
		}
		else {
			//auto error = SDL_GetError();
			SDL_ShowSimpleMessageBox(0, "error", SDL_GetError(), window);
		}

		auto r = SDL_Rect{
			pos.x()*res*CHUNK::width,
			pos.y()*res*CHUNK::height,
			CHUNK::width*res,
			CHUNK::height*res
		};
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		draw(cachedChunk, r);
		//SDL_RenderCopy(renderer, cachedChunk, NULL, &r);
		//SDL_RenderDrawRect(renderer, &r);

	}
};

/*--------------------------------------------------------*/

CTOR() : p{ new Impl() } {
	std::string error = SDL_GetError();
	SDL_SetRenderDrawBlendMode(p->renderer, SDL_BLENDMODE_BLEND);

	int screenW = 0;
	int screenH = 0;
	SDL_GetWindowSize(p->window, &screenW, &screenH);
	originX = screenW / 2;
	originY = screenH / 2;
	offsetX = 0;
	offsetY = 100;
}
DTOR() { SDL_DestroyRenderer(p->renderer); SDL_DestroyWindow(p->window); }

//template <typename T>
void draw(const int& key, int x, int y)
{
	if (key == 0)
		return;
	auto location = p->find(key);
	p->draw(location,x,y);
}

//template <typename T>
void draw(const std::string& key, int x, int y)
{
	auto location = p->find(key);
	p->draw(location,x,y);
}

void draw(const AA::Pos& pos, char& id)
{
	auto& asset = p->find(id);
	p->draw(asset, pos.x(), pos.y());
}

void clear()
{
	SDL_SetRenderDrawColor(p->renderer, 60, 60, 60, 255);
	SDL_RenderClear(p->renderer);
}

void drawChunk(unique_ptr<_sim>& sim, const AA::Pos& pos)
{
	std::string chunkData = sim->getChunk(pos);
	p->drawChunk(chunkData, pos);
}

void loadAsset(const std::string& path, int key)
{
	p->loadAsset(path,key);
}

void present()
{
	SDL_RenderPresent(p->renderer);
}