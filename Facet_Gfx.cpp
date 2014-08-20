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
	using search_result = container_type::iterator;
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

	Impl(scalar size = scalar(1024,768), bool relativeToScreen = false)
	{
		SDL_DisplayMode screen;
		SDL_GetCurrentDisplayMode(0, &screen);

		if (relativeToScreen){
			screen.w = static_cast<int>(screen.w*(size.x / 100));
			screen.h = static_cast<int>(screen.h*(size.y / 100));
		}
		else {
			screen.w = static_cast<int>(size.x);
			screen.h = static_cast<int>(size.y);
		}

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
		int preferredSize = static_cast<int>(res*zoom);

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
		return selected;
	}

	void draw(potential_result const& item, const scalar& location){

		if (item.get_ptr() == NULL)
			throw std::range_error("Search for item failed!");

		scalar view = Transform(location, SIMSPACE, CAMERASPACE);

		int size = static_cast<int>(res*zoom);

		auto& asset = item.get()->second;
		asset.draw(static_cast<int>(view.x), static_cast<int>(view.y), size, size, true);

	}

	//screen starts at 0,0. Camera starts in the middle of the screen.
	enum SPACE {
		SCREENSPACE,
		CAMERASPACE,
		SIMSPACE,
	};

	scalar Transform(const scalar& pos, SPACE cur, SPACE dest)
	{
		scalar p1 = pos;
		if (cur == SIMSPACE && dest == SCREENSPACE)
		{
			p1 *= res*zoom;
			p1 += offset * zoom;
			return p1;
		}
		if (cur == SCREENSPACE && dest == SIMSPACE)
		{
			p1 -= offset*zoom;
			p1 /= res*zoom;
			return p1;
		}
		if (cur == SIMSPACE && dest == CAMERASPACE)
		{
			p1 = (Transform(p1, SIMSPACE, SCREENSPACE) + origin);
			return p1;
		}
		if (cur == CAMERASPACE && dest == SIMSPACE) //offset calculations to screenspace by converting to it first.
		{
			p1 = (Transform(p1, SCREENSPACE, SIMSPACE) - origin);
			return p1;
		}
		throw std::runtime_error("No correct transform found");
	}

	void drawAround(const scalar& pos){

		scalar view = pos;
		view -= offset;
		view *= res*zoom;
		view += offset + origin;

		int size = static_cast<int>(res*zoom);

		SDL_Rect rect = { static_cast<int>(view.x), static_cast<int>(view.y), size, size };

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

CTOR(scalar size = scalar(1024,768), bool relativeToScreen ) : p{ new Impl(size, relativeToScreen) } {
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
	p->offset += dp / p->zoom;
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
	try {
		for (auto item : session->get_child("Config.Entities"))
		{
			auto id = item.second.get<char>("ID");
			for (auto asset : item.second.get_child_optional("Assets").get_value_or(ptree()))
			{
				auto name = asset.first;
				auto path = asset.second.get<std::string>("Path");
				if (name == "Normal")
					p->loadAsset(path, id);
				else if (name == "small")
					p->loadSmallAsset(path, id);
			}
			/*auto mainAsset = item.second.get<std::string>("Assets");
			char key = item.second.data()[0];
			p->loadAsset(mainAsset, key);

			auto smallAsset = item.second.get_optional<std::string>("asset.small").get_value_or("");
			if (smallAsset != "")
				p->loadSmallAsset(smallAsset, key);*/
		}
	}
	catch (std::exception e)
	{
		SDL_ShowSimpleMessageBox(0, "GFX Init Error", e.what(), NULL);
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
			auto& groupKey = group->first;
			for (auto& stack : group->second)
			{
				auto& pos = stack.first;
				for (auto& item : stack.second)
				{
					if (item == nullptr)
						continue;
					if (groupKey == tile::FIREGROUP)
					{
						tile::Fire* f = reinterpret_cast<tile::Fire*>(item);
						SDL_Color c = SDL_Color{ (f->regionID / 3 * 25) + ((f->regionID % 3) == 0 ? 155 : 0), +((f->regionID % 3) == 1 ? 155 : 0), +((f->regionID % 3) == 0 ? 155 : 0), 255 };
						highlightCell(pos, c);
					}

					if (groupKey == tile::GEOGRAPHYGROUP) {
						tile::Land* f = reinterpret_cast<tile::Land*>(item);
						int h = f->elevation;
						if (f->isWater())
							fillCell(pos, SDL_Color{ 0, 0, 255, 255 });
						continue;
					}
					else {
						auto& id = item->id;
						draw(id, pos);
					}
				}
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
	for (auto& stack : data[tile::UNITGROUP])
	{
		auto& pos = stack.first;
		for (auto& item : stack.second)
		{
			//auto& res = (*p->find(item.second->id))->second;
			//int size = res.getSize();
			//SDL_Rect rect{ pos.x - size, pos.y - size, size * 3, size * 3 };
			//p->drawAround(scalar(0,0));
		}
	}
	SDL_SetRenderDrawBlendMode(p->renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(p->renderer, NULL);
	SDL_SetTextureBlendMode(p->fog, SDL_BLENDMODE_BLEND);
	//SDL_RenderCopy(p->renderer, p->fog, NULL, NULL);
}

void CLASS::drawOverview(master_type& data)
{
	for (const tile::id_type& id : tile::group_order)
	{
		auto& group = data.find(id);
		if (group != end(data))
		{
			auto& groupKey = group->first;
			//for (auto& stack : group->second)
			auto screen = p->screenSize * p->res / p->zoom * 0.1;
			auto offset = p->Transform(p->offset, p->SCREENSPACE, p->SIMSPACE)*0;
			auto start = offset - screen;
			auto end = offset + screen;
			//auto scart = screen
			auto& plane = group->second;
			for (int y = start.y; y < end.y; y++) {
				for (auto stack = plane.lower_bound(scalar(start.x, y)); stack != plane.lower_bound(scalar(end.x, y)); ++stack)
				{
					auto& pos = stack->first;
					for (auto& item : stack->second)
					{
						if (item == nullptr)
							continue;
						if (groupKey == tile::FIREGROUP)
						{
							tile::Fire* f = reinterpret_cast<tile::Fire*>(item);
							SDL_Color c = SDL_Color{
								(f->regionID / 3 * 25) + ((f->regionID % 3) == 0 ? 155 : 0) + 128,
								((f->regionID % 3) == 1 ? 155 : 0),
								((f->regionID % 3) == 0 ? 155 : 0),
								128 };
							auto& id = item->id;
							auto asset = p->find(id);
							if (f->isRoot) {
								if (asset.is_initialized())
								{
									auto camPos = p->Transform(pos, Impl::SIMSPACE, IMPL::CAMERASPACE);
									highlightCell(pos, c);
									asset.get()->second.draw(camPos.x, camPos.y, 64, 64, true);
								}
							}
							else {
								highlightCell(pos, c);
							}
						}

						if (groupKey == tile::GEOGRAPHYGROUP) {
							tile::Land* f = reinterpret_cast<tile::Land*>(item);
							int h = f->elevation;
							h *= 4;
							if (h < 0) h = 0;
							if (h > 255) h = 255;
							if (f->isWater())
								fillCell(pos, SDL_Color{ 0, 0, 255, 64 });
							else if (h < 200) {
								fillCell(pos, SDL_Color{ h*1.8, 64 - h, h*1.2, 64 - h / 4 });
							}
							else {
								fillCell(pos, SDL_Color{ 255, 255, 255, 64 + (h - 200) * 5 });
							}
						}
						if (groupKey == tile::OBJECTGROUP) {
							fillCell(pos, SDL_Color{ 0, 255, 0, 5 });
						}
						if (groupKey == tile::UNITGROUP) {
							auto& id = item->id;
							draw(id, pos);
						}

					}
				}
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
	for (auto& stack : data[tile::UNITGROUP])
	{
		auto& pos = stack.first;
		for (auto item : stack.second)
		{
			//auto& res = (*p->find(item.second->id))->second;
			//int size = res.getSize();
			//SDL_Rect rect{ pos.x - size, pos.y - size, size * 3, size * 3 };
			//p->drawAround(scalar(0,0));
		}
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

void CLASS::draw(const char id, const scalar& pos)
{
	auto& query = p->find(id);
	if(query.is_initialized())
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
	return p->Transform(mousePos - p->origin, p->SCREENSPACE, p->SIMSPACE);
}

double CLASS::getZoom()
{
	return p->zoom;
}

void CLASS::highlightCell(const scalar& cell, SDL_Color& col)
{
	scalar view = cell;
	view.x = round(view.x);
	view.y = round(view.y);

	//view = view - view % p->res - p->offset*p->res;
	view = p->Transform(view, p->SIMSPACE, p->SCREENSPACE);
	int size = static_cast<int>(p->res*p->zoom);

	auto r = SDL_Rect{ static_cast<int>(view.x - size / 2 + p->origin.x), static_cast<int>(view.y - size / 2 + p->origin.y), size, size };
	SDL_SetRenderDrawColor(p->renderer, col.r, col.g, col.b, col.a);
	SDL_RenderDrawRect(p->renderer, &r);
	SDL_SetRenderDrawColor(p->renderer, col.r, col.g, col.b, 32);
	SDL_RenderFillRect(p->renderer, &r);
}

void CLASS::fillCell(const scalar& cell, SDL_Color& col)
{
	scalar view = cell;
	view.x = round(view.x);
	view.y = round(view.y);

	//view = view - view % p->res - p->offset*p->res;
	view = p->Transform(view, p->SIMSPACE, p->SCREENSPACE);
	int size = static_cast<int>(p->res*p->zoom);

	auto r = SDL_Rect{ static_cast<int>(view.x - size / 2 + p->origin.x), static_cast<int>(view.y - size / 2 + p->origin.y), size, size };
	SDL_SetRenderDrawColor(p->renderer, col.r, col.g, col.b, 32);
	SDL_RenderFillRect(p->renderer, &r);
}