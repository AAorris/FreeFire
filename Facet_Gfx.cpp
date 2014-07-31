#include "stdafx.h"
#include "Facet_Gfx.h"
#include <algorithm>
#include <unordered_map>
#include "camera_data.h"

#define CLASS Facet_Gfx
#include "PIMPL.h"

struct IMPL {
	using container_type = std::unordered_map<char, _asset>;
	using search_type = char;
	using search_result = container_type::iterator&;
	using potential_result = boost::optional<search_result>;
	//typedef std::unordered_set< _asset >	container_type;
	//typedef container_type::iterator		container_iterator;
	//typedef const _asset&					container_value;

	//std::unordered_map<AA::Pos, sdl_uptr<SDL_Texture>> cached_chunks;

	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* fog;

	container_type assets;
	container_type smallAssets;

	double res;
	scalar offset;
	scalar origin;
	scalar screenSize;
	double zoom;

	Impl(scalar size = scalar(1024,768))
	{
		SDL_DisplayMode screen;
		SDL_GetCurrentDisplayMode(0, &screen);

		screen.w = size.x;
		screen.h = size.y;

		screenSize = scalar{ double(screen.w), double(screen.h) };

		res = 32;
		offset.x = 0;
		offset.y = 0;
		zoom = 1;
		origin.x = screen.w / 2;
		origin.y = screen.h / 2;

		window = SDL_CreateWindow("Free Fire 1.7.24", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen.w, screen.h,0);
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
		fog = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, screen.w, screen.h);
		assets = container_type{};
		smallAssets = container_type{};
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

		potential_result selected{};
		if (zoom < 1)
		{
			auto& result = smallAssets.find(id);
			if (result != end(smallAssets))
				selected = result;
		}
		if (selected.is_initialized() == false)
		{
			auto& result = assets.find(id);
			if (result != end(assets))
				selected = result;
		}		
		/*signed int selectedDistance = -1;

		if (std::distance(result.first, result.second) > 0) {
			for (auto it = result.first; it != result.second; ++it)
			{
				int distance = abs(it->second.getSize() - preferredSize);
				if (distance < selectedDistance)
				{
					selectedDistance = distance;
					selected = it;
				}
				if (distance == 0)
					break;
			}
		}*/
		return selected;
		//auto test = [n](container_value i) { return i.id == n; };
		//return std::find_if(assets.begin(), assets.end(), test);
	}

	void draw(potential_result const& item, const scalar& location){

		if (!item.is_initialized())
			throw std::range_error("Search for item failed!");

		scalar view{ double(location.x), double(-location.y) };
		view -= offset;
	//	view *= res*zoom;
		view *= res;
		view += offset+origin;

		int size = res*zoom;

		(*item)->second.draw(view.x, view.y, size, size);

	}

	void drawAround(const scalar& pos){

		scalar view = pos;
		view -= offset;
		view *= res*zoom;
		view += offset + origin;

		double size = res*zoom;

		SDL_Rect rect = {view.x, view.y, size, size};

		SDL_RenderFillRect(renderer,&rect);
		//(*item)->second.draw(view.x, view.y, size, size);

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

	void loadSmallAsset(const std::string& s, const char& id)
	{
		smallAssets.insert(std::make_pair(
			id,
			_asset(s, renderer)
			)
		);
	}
};

/*--------------------------------------------------------*/

CTOR(scalar size = scalar(1024,768)) : p{ new Impl(size) } {
	std::string error = SDL_GetError();
	SDL_SetRenderDrawBlendMode(p->renderer, SDL_BLENDMODE_BLEND);
}
DTOR() { SDL_DestroyRenderer(p->renderer); SDL_DestroyWindow(p->window); }

const _asset* CLASS::getAsset(const tile::id_type& key)
{
	return &p->assets.at(static_cast<char>(key));
	/*
	auto& res = p->find(key);
	if (res.is_initialized()){
		auto& asset = res.get()->second;
		return &asset;
	}
	else
		return NULL;
	*/
}

void CLASS::zoomCamera(const double& dz)
{
	p->zoom += dz;
	p->zoom = SDL_max(p->zoom,4.0/p->res); //zoom should not be lower than 1.0/res
}
void CLASS::moveCamera(const scalar& dp)
{
	p->offset += dp/(p->res*p->zoom);
}
void CLASS::resize(int x, int y)
{
	p->screenSize = scalar(x, y);
	SDL_Rect r = { 0, 0, x, y };
	SDL_RenderSetClipRect(p->renderer, NULL);
}
cameraTool_Data CLASS::getCamera()
{
	return cameraTool_Data(p->origin, p->offset, p->screenSize, p->zoom, p->res);
}

void CLASS::connect(_cfg& session)
{
	for (auto item : session->get_child("Templates"))
	{
		auto mainAsset = item.second.get<std::string>("asset");
		char key = item.second.data()[0];
		p->loadAsset(mainAsset, key);

		auto smallAsset = item.second.get_optional<std::string>("asset.small").get_value_or("");
		if (smallAsset!="")
			p->loadSmallAsset(smallAsset, key);
	}
}

void Facet_Gfx::draw(SDLButton* button)
{
	SDLButton& b = *button;
	SDL_Renderer* ren = p->renderer;

	//drawing background
	SDL_Rect bg = { b.x, b.y, b.w, b.h };
	SDL_Color bgCol = { 255, 0, 0, 255 };
	if (b.clicking)
		SDL_SetRenderDrawColor(ren, b.clickCol.r, b.clickCol.g, b.clickCol.b, b.clickCol.a);
	else if (b.hovering)
		SDL_SetRenderDrawColor(ren, b.hoverCol.r, b.hoverCol.g, b.hoverCol.b, b.hoverCol.a);
	else
		SDL_SetRenderDrawColor(ren, b.idleCol.r, b.idleCol.g, b.idleCol.b, b.idleCol.a);
	SDL_RenderFillRect(ren, &bg);

	//drawing label (?)
	if (b.textLabel != "")
	{
		if (b.t_label != NULL)
			SDL_RenderCopy(ren, b.t_label, NULL, &b.r_label);
		else
		{
			//SDL_Texture* tex = getText(b.textLabel, "robotoblack12");
			//b.t_label = tex;
			int tw = 0;
			int th = 0;
			//SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
			//line up the two centers
			// c = center
			// b = background
			// t = text
			int cbx = b.x + b.w / 2;
			int cby = b.y + b.h / 2;
			int ctx = b.x + tw / 2;
			int cty = b.y + th / 2;
			//offset from normal position is the difference
			//between their centers.
			int r_x = b.x + (cbx - ctx);
			int r_y = b.y + (cby - cty);

			b.r_label = { r_x, r_y, tw, th };
			//SDL_RenderCopy(ren, b.t_label, NULL, &b.r_label);
		}
	}
}

void Facet_Gfx::draw(master_type& data)
{
	for (const tile::id_type& id : tile::group_order)
	{
		auto& group = data.find(id);
		if (group != end(data))
		{
			for (auto& item : group->second)
			{
				auto& id = item.second->id;
				auto& pos = item.first;
				draw(id, pos);

			}
		}
	}
	SDL_SetRenderTarget(p->renderer, p->fog);
	SDL_SetTextureBlendMode(p->fog, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawBlendMode(p->renderer, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(p->renderer, 0, 0, 0, 180);
	SDL_RenderClear(p->renderer);
	SDL_SetRenderDrawColor(p->renderer, 0, 0, 0, 0);
	//fog of war
	for (auto& item : data[tile::UNITGROUP])
	{
		auto& pos = item.first;
		//auto& res = (*p->find(item.second->id))->second;
		//int size = res.getSize();
		//SDL_Rect rect{ pos.x - size, pos.y - size, size * 3, size * 3 };
		//p->drawAround(scalar(0,0));
	}
	SDL_SetRenderDrawBlendMode(p->renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(p->renderer, NULL);
	SDL_SetTextureBlendMode(p->fog, SDL_BLENDMODE_BLEND);
	//SDL_RenderCopy(p->renderer, p->fog, NULL, NULL);
}

std::pair<SDL_Window*, SDL_Renderer*> CLASS::context()
{
	return{ p->window, p->renderer };
}

void CLASS::draw(const char& id, const scalar& pos)
{
	auto& query = p->find(id);
	assert(query.is_initialized());
	p->draw(query, pos);
}

void CLASS::clear()
{
	SDL_SetRenderDrawColor(p->renderer, 65, 60, 60, 255);
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

scalar CLASS::getCell(const scalar& mousePos)
{
	//transform mouse origin to screen's center
	scalar distanceFromCenter = mousePos - p->origin;
	scalar distanceFromOrigin = scalar(0, 0);// p->offset;
	int cellSize = p->res*p->zoom;
	double zoom = p->zoom;

	scalar finalPos = ( distanceFromCenter / zoom + distanceFromOrigin ) / cellSize;
	//finalPos -= p->offset + p->origin;
	//finalPos /= p->res*p->zoom;
	//finalPos += p->offset;
	//finalPos.x = finalPos.x - (static_cast<int>(finalPos.x) % (int)(p->res*p->zoom));
	//finalPos.y = finalPos.y - (static_cast<int>(finalPos.y) % (int)(p->res*p->zoom));
	//finalPos.y *= -1;
	//finalPos /= p->res * p->zoom;
	return p->offset;
}
 
void CLASS::highlightCell(const scalar& cell)
{
	scalar view{ double(cell.x), double(-cell.y) };
	view -= p->offset;
	view *= p->res*p->zoom;
	view += p->offset + p->origin;

	auto r = SDL_Rect{ view.x, view.y, p->res*p->zoom, p->res*p->zoom };
	SDL_SetRenderDrawColor(p->renderer, 255, 0, 0, 255);
	SDL_RenderDrawRect(p->renderer, &r);
}