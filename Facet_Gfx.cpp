#include "stdafx.h"
#include "Facet_Gfx.h"
#include <algorithm>
#include <unordered_map>
#include "camera_data.h"

#define CLASS Facet_Gfx
#include "PIMPL.h"

struct IMPL {
	using container_type = std::unordered_multimap<char, _asset>;
	using search_type = std::pair<container_type::iterator, container_type::iterator>;
	using search_result = container_type::iterator;
	using potential_result = boost::optional<search_result>;
	//typedef std::unordered_set< _asset >	container_type;
	//typedef container_type::iterator		container_iterator;
	//typedef const _asset&					container_value;

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

	/*Get a tile's type and find it's asset pool.*/
	potential_result find(const char& id)
	{
		int preferredSize = res*zoom;

		search_type result = assets.equal_range(id);
		
		potential_result selected{};
		unsigned int selectedDistance = -1;

		if (std::distance(result.first, result.second) > 0) {
			for (auto it = result.first; it != result.second; ++it)
			{
				int distance = abs(it->second.getSize() - preferredSize);
				if (distance < selectedDistance)
				{
					selectedDistance = distance;
					selected = it;
				}
			}
		}
		return selected;
		//auto test = [n](container_value i) { return i.id == n; };
		//return std::find_if(assets.begin(), assets.end(), test);
	}

	void draw(potential_result const& item, const scalar& location){

		if (!item.is_initialized())
			throw std::range_error("Search for item failed!");

		scalar view{ double(location.x), double(-location.y) };
		view -= offset;
		view *= res*zoom;
		view += offset+origin;

		int size = res*zoom;

		(*item)->second.draw(location.x, location.y, size, size);

	}

	void loadAsset(const std::string& s, char id){
		assets.insert(
			std::make_pair(
				id,
				_asset(s, renderer)
			)
		);
		//assets.insert(_asset(s, renderer, id));
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
cameraTool_Data CLASS::getCamera()
{
	return cameraTool_Data(p->origin, p->offset, p->screenSize, p->zoom, p->res);
}

void CLASS::connect(Tool_Data* to)
{
	if (p->assets.count(to->id) == 0)
	{
		for (auto path : to->assets)
		{
			char type = to->id;
			p->loadAsset(path, type);
		}
	}
}

void CLASS::draw(Tool_Data* data)
{
	auto& pos = data->pos;
	int size = p->res * p->zoom;
	auto query = p->find(data->id);
	p->draw(query, pos*size);
}

void CLASS::draw(const char& id, const scalar& pos)
{
	auto& query = p->find(id);
	p->draw(query, pos);
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
 
