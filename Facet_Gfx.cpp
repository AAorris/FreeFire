#include "stdafx.h"
#include "Facet_Gfx.h"
#include <algorithm>
#include <unordered_map>
#include "camera_data.h"

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

	double res;
	scalar offset;
	scalar origin;
	scalar screenSize;
	double zoom;

	Impl()
	{
		SDL_DisplayMode screen;
		SDL_GetCurrentDisplayMode(0, &screen);

		screen.w *= 1;
		screen.h *= 1;

		screenSize = scalar{ double(screen.w), double(screen.h) };

		res = 32;
		offset.x = 0;
		offset.y = 0;
		zoom = 1;
		origin.x = screen.w / 2;
		origin.y = screen.h / 2;

		window = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen.w, screen.h, 0);
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
		auto r = SDL_Rect{ 0, 0, screen.w, screen.h };
		SDL_RenderSetClipRect(renderer, &r);
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
		if (window != 0)
			SDL_DestroyWindow(window);
		if (renderer != 0)
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

	void draw(container_iterator it, int x = 0, int y = 0){
		scalar view{ double(x), double(-y) };

		view -= offset;
		view *= res*zoom;
		view += offset+origin;

		if (it != assets.end())
			it->draw(view.x, view.y,res*zoom,res*zoom); 
	}

	void loadAsset(const std::string& s, int id){
		assets.insert(_asset(s, renderer, id));
	}
};

/*--------------------------------------------------------*/

CTOR() : p{ new Impl() } {
	std::string error = SDL_GetError();
	SDL_SetRenderDrawBlendMode(p->renderer, SDL_BLENDMODE_BLEND);
}
DTOR() { SDL_DestroyRenderer(p->renderer); SDL_DestroyWindow(p->window); }

void CLASS::zoomCamera(const double& dz)
{
	p->zoom += dz;
	p->zoom = SDL_max(p->zoom,4.0/p->res); //zoom should not be lower than 1.0/res
}
void CLASS::moveCamera(const scalar& dp)
{
	p->offset += dp/(p->res*p->zoom);
}
camera_data CLASS::getCamera()
{
	return camera_data(p->origin, p->offset, p->screenSize, p->zoom, p->res);
}
//template <typename T>
void CLASS::draw(const int& key, int x, int y)
{
	if (key == 0)
		return;
	auto location = p->find(key);
	p->draw(location,x,y);
}

//template <typename T>
void CLASS::draw(const std::string& key, int x, int y)
{
	auto location = p->find(key);
	p->draw(location,x,y);
}

void CLASS::draw(const AA::Pos& pos, char& id)
{
	auto& asset = p->find(id);
	if (asset != end(p->assets))
		p->draw(asset, pos.x(), pos.y());
}

void CLASS::clear()
{
	SDL_SetRenderDrawColor(p->renderer, 60, 60, 60, 255);
	SDL_RenderClear(p->renderer);
}

void CLASS::loadAsset(const std::string& path, int key)
{
	p->loadAsset(path,key);
}

void CLASS::present()
{
	SDL_RenderPresent(p->renderer);
}
 
