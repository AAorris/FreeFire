#include "stdafx.h"
#include "Facet_Gfx.h"
#include <algorithm>
#include <unordered_map>
#include <SDL2\SDL_surface.h>
#include <SDL2\SDL_image.h>
#include "camera_data.h"
#include "Geometry.h"

#define CLASS Facet_Gfx
#include "PIMPL.h"

SDL_Texture* IMG_LoadTexture(SDL_Renderer* renderer, const std::string& path) { return IMG_LoadTexture(renderer, path.c_str()); }
SDL_Surface* IMG_Load(const std::string& file) { return IMG_Load(file.c_str()); }

SDL_Texture* LoadTexture(SDL_Renderer* renderer, const std::string& path) { return IMG_LoadTexture(renderer, path.c_str()); }
SDL_Surface* LoadSurface(const std::string& file) { return IMG_Load(file.c_str()); }
SDL_Texture* CreateTextureTarget(SDL_Renderer* r, const int& w, const int& h) { return SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h); }
void SetColor(SDL_Renderer* r, SDL_Color c) {
	SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
}
SDL_Color BlendColor(SDL_Color& c1, SDL_Color& c2, double amt=0.5)
{
	auto avg = [&amt](int a, int b) { return a * (1 - amt) + b*(amt); };
	return SDL_Color{ avg(c1.r, c2.r), avg(c1.g, c2.g), avg(c1.b, c2.b), avg(c1.a, c2.a) };
}
void SetTarget(SDL_Renderer* r, SDL_Texture* t = NULL) { SDL_SetRenderTarget(r, t); }
void ResetTarget(SDL_Renderer* r) { SetTarget(r); }
int Render(SDL_Renderer* r, SDL_Texture* t, SDL_Rect* src, SDL_Rect* dst) { return SDL_RenderCopy(r, t, src, dst); }

struct IMPL {
	using container_type = std::unordered_map<char, _asset>;
	using search_type = char;
	using search_result = container_type::iterator;
	using potential_result = boost::optional<search_result>;

	SDL_Surface* fireColors;
	//typedef std::unordered_set< _asset >	container_type;
	//typedef container_type::iterator		container_iterator;
	//typedef const _asset&					container_value;

	//std::unordered_map<AA::Pos, sdl_uptr<SDL_Texture>> cached_chunks;

	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* fog;

	SDL_Texture* geographyTexture;
	SDL_Texture* tileMask;
	SDL_Texture* alphaBuffer;
	SDL_Texture* colorBuffer;
	scalar terrainOffset;
	std::vector<SDL_Texture*> heightTextures;
	PerlinNoise perlin;

	void initGeographyMasking(std::string maskPath, std::vector<std::string> texturePaths)
	{
		tileMask = IMG_LoadTexture(renderer, maskPath);
		//assert(SDL_SetTextureColorMod(tileMask, 0, 0, 0) != -1);
		alphaBuffer = CreateTextureTarget(renderer, 32 * 3, 32 * 3);
		for (auto& path : texturePaths)
			heightTextures.push_back(IMG_LoadTexture(renderer, path));
		//SDL_SetTextureBlendMode(tileBuffer, SDL_BLENDMODE_MOD);
	}

	int toint(double d) { return static_cast<int>(d); }

	/*px and py are position hints for where in the world this terrain tile is rendering. Height is how high the tile is.
	The goal is to render to the terrain tile buffer(alphaBuffer), and use that afterwards to render the tile. This should
	handle blending between height and such.*/
	void bufferTerrain(double px, double py, double height) {
		SetTarget(renderer, alphaBuffer);
		std::vector<SDL_Color> colors{
			{ 0, 0, 0, 255 },
			{ 32, 24, 1, 255 },
			{ 32, 64, 5, 255 },
			{ 40, 80, 20, 255 },
			{ 80, 100, 60, 255 },
			{ 128, 128, 128, 255 },
			{ 255, 255, 255, 255 },
			{ 255, 255, 255, 255 }
		};
		int maxIndex = colors.size() - 1;
		double ind = height * maxIndex;
		int find = toint(floor(height*maxIndex));
		int cind = toint(ceil(height * maxIndex));
		double blend = cind - (ind);
		const double flatSize = 0.5 * 0.1;
		
		//blend range
		if (blend > flatSize && blend < (1 - flatSize)) {
			if (cind < colors.size())
			{
				SDL_Color blended = BlendColor(colors[find], colors[cind], 1 - ((blend - flatSize) / ((1 - flatSize) - flatSize)));
				SetColor(renderer, blended);
				SDL_RenderFillRect(renderer, NULL);
				if (height <= (tile::Land::waterLevel + 128) / 255.0){
					SDL_SetRenderDrawColor(renderer, 15, 64, 128, 64);
					SDL_RenderFillRect(renderer, NULL);
				}
			}
			else if (cind == colors.size()) {
				SDL_Color blended = BlendColor(colors[find], colors[find], (blend - flatSize) / (1 - flatSize));
				SetColor(renderer, blended);
				SDL_RenderFillRect(renderer, NULL);
				if (height <= (tile::Land::waterLevel + 128) / 255.0){
					SDL_SetRenderDrawColor(renderer, 15, 64, 128, 64);
					SDL_RenderFillRect(renderer, NULL);
				}
			}
		}
		//not in blend range : FLOOR IT
		else {
			SDL_Color blended = BlendColor(colors[cind], colors[find], round(blend));
			SetColor(renderer, blended);
			SDL_RenderFillRect(renderer, NULL);
			if (height <= (tile::Land::waterLevel + 128) / 255.0){
				SDL_SetRenderDrawColor(renderer, 15, 64, 128, 64);
				SDL_RenderFillRect(renderer, NULL);
			}
		}
		///SetColor(renderer, { 255, 0, 0, 255 });
		//SDL_RenderDrawRect(renderer, NULL);
		SetTarget(renderer);
	}

	SDL_Texture* createTerrainTexture(facet::master_group_type* terrain, facet::master_group_type* objects)
	{
		int period = 1000 / 12;
		int goal = SDL_GetTicks() + period;

		auto first = terrain->begin()->first;
		auto last = terrain->rbegin()->first;// terrain->end()--->first; // that's a -- and a ->
		int w = last.x - first.x;
		int h = last.y - first.y;
		SDL_Texture* texture = CreateTextureTarget(renderer, w*res, h*res);
		SetTarget(renderer, texture);
		SDL_Rect clip = SDL_Rect{ 0, 0, w*res, w*res };
		SDL_RenderSetClipRect(renderer, &clip);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderFillRect(renderer, NULL);
		SetTarget(renderer);
		for (auto& stack : *terrain)
		{
			auto pos = stack.first;
			scalar realPos = (stack.first - first)*res;
			SDL_Rect r = SDL_Rect{
				realPos.x - res,
				realPos.y - res,
				res * 3,
				res * 3
			};
			for (auto& item : stack.second)
			{
				tile::Land* land = reinterpret_cast<tile::Land*>(item);
				double h = (land->elevation)/255.0+0.5;
				int hCol = static_cast<int>(h * 255);
				hCol = (hCol<0)?0:(hCol>255)?255:hCol;
				bufferTerrain(realPos.x, realPos.y, h);
				SDL_SetTextureBlendMode(alphaBuffer, SDL_BLENDMODE_BLEND);
				SDL_SetTextureAlphaMod(alphaBuffer, 255/4);
				SetTarget(renderer, texture);
				SDL_RenderSetClipRect(renderer, &clip);
				Render(renderer, alphaBuffer, NULL, &r);
				if (h < 0.15){
					SDL_SetTextureColorMod(texture, 255, 255, 255);
					SDL_SetRenderDrawColor(renderer, 0, 64, 255, 128/8);
					SDL_RenderFillRect(renderer, &r);
				}
				SetTarget(renderer);
			}
			if (SDL_GetTicks() > goal)
			{
				goal += period;
				Render(renderer, texture, NULL, NULL);
				SDL_RenderPresent(renderer);
			}
		}
		SetTarget(renderer, texture);
		SDL_RenderSetClipRect(renderer, &clip);
		scalar base = objects->begin()->first;
		for (auto& stack : *objects) {
			for (auto& item : stack.second) {
				SDL_Rect rect = SDL_Rect{ (stack.first.x - base.x)*res + res/4, (stack.first.y - base.y)*res + res / 4, res*0.5, res*0.5 };
				
				draw(find(item->id), scalar(0,0), &rect);
				//draw(find(item->id), stack.first);
			}
		}
		SDL_Rect screen = SDL_Rect{ 0, 0, screenSize.x, screenSize.y };
		SetTarget(renderer);
		SDL_RenderSetClipRect(renderer, &screen);
		Render(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		//SDL_Delay(15000);
		terrainOffset = scalar(-w / 2, -h / 2);
		return texture;
	}

	void destroyGeographyMasking()
	{
		for (auto item : heightTextures)
			SDL_DestroyTexture(item);
		SDL_DestroyTexture(tileMask);
		SDL_DestroyTexture(alphaBuffer);
		SDL_DestroyTexture(geographyTexture);
	}

	//void 

	container_type assets;
	container_type smallAssets;

	double res;
	scalar offset;
	scalar origin;
	scalar screenSize;
	double zoom;
	Impl(scalar size = scalar(1024, 768), bool relativeToScreen = false)
	{
		SDL_DisplayMode screen;
		SDL_GetCurrentDisplayMode(0, &screen);

		fireColors = IMG_Load("assets/fireColors.png");

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

		window = SDL_CreateWindow("Free Fire 1.7.24", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen.w, screen.h, 0);
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
		perlin = PerlinNoise(rand());
		initGeographyMasking("assets/heightmaps/alphaMask.tga", {
			"assets/heightmaps/0.jpg",
			"assets/heightmaps/1.jpg",
			"assets/heightmaps/2.jpg",
			"assets/heightmaps/3.jpg",
			"assets/heightmaps/4.jpg",
			"assets/heightmaps/5.jpg", });
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

	void draw(potential_result const& item, const scalar& location, SDL_Rect* rect = nullptr){

		if (item.get_ptr() == NULL)
			throw std::range_error("Search for item failed!");

		scalar view = Transform(location, SIMSPACE, CAMERASPACE);

		int size = static_cast<int>(res*zoom);

		auto& asset = item.get()->second;

		if (rect == nullptr)
			asset.draw(static_cast<int>(view.x), static_cast<int>(view.y), size, size, true);
		else
			asset.draw(rect);

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
		if (cur == SCREENSPACE && dest == CAMERASPACE)
		{
			p1 += offset*zoom;
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

		SDL_RenderFillRect(renderer, &rect);
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

CTOR(scalar size = scalar(1024, 768), bool relativeToScreen) : p{ new Impl(size, relativeToScreen) } {
	std::string error = SDL_GetError();
	SDL_SetRenderDrawBlendMode(p->renderer, SDL_BLENDMODE_BLEND);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	p->loadSmallAsset("assets/incident.png", 0xF0);
	p->loadSmallAsset("assets/fire-small.png", 0xF1);
	p->loadAsset("assets/incident.png", 0xF0);
	p->loadAsset("assets/fire-small.png", 0xF1);
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
	p->zoom = SDL_max(p->zoom, 4.5 / p->res); //zoom should not be lower than 1.0/res
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
	p->loadAsset("assets/fire.png", 'F');
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
		for (auto item : session->get_child("Config.Tiles"))
		{
			auto id = item.second.get_optional<char>("ID");
			if (id.is_initialized()) {
				for (auto asset : item.second.get_child_optional("Assets").get_value_or(ptree()))
				{
					auto name = asset.first;
					auto path = asset.second.get<std::string>("Path");
					if (name == "Normal")
						p->loadAsset(path, *id);
					else if (name == "small")
						p->loadSmallAsset(path, *id);
				}
			}
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
						int index = (std::hash<std::string>()(f->region) + std::hash<int>()(f->regionID)) % p->fireColors->w;
						SDL_LockSurface(p->fireColors);
						Uint8* pixel = reinterpret_cast<Uint8*>(reinterpret_cast<Uint32*>(p->fireColors->pixels) + index);
						Uint8 r = *(pixel + 0);
						Uint8 g = *(pixel + 1);
						Uint8 b = *(pixel + 2);
						Uint8 a = *(pixel + 3);
						SDL_UnlockSurface(p->fireColors);
						SDL_Color c = SDL_Color{ r, g, b, a };
						highlightCell(pos, c);
					}

					else if (groupKey == tile::GEOGRAPHYGROUP) {
						tile::Land* f = reinterpret_cast<tile::Land*>(item);
						int h = f->elevation;
						if (f->isWater())
							fillCell(pos, SDL_Color{ 0, 0, 255, 255 });
						continue;
					}
					else if (groupKey == tile::OBJECTGROUP) {
						if (item->burnable == false)
							fillCell(pos, SDL_Color{ 255, 128, 0, 128 });
						auto& id = item->id;
						draw(id, pos);
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

	scalar firstCell = data[tile::GEOGRAPHYGROUP].begin()->first;
	scalar lastCell = data[tile::GEOGRAPHYGROUP].rbegin()->first;


	scalar realFirst = p->Transform(firstCell, p->SIMSPACE, p->CAMERASPACE);
	scalar realLast = p->Transform(lastCell, p->SIMSPACE, p->CAMERASPACE);

	int x = realFirst.x - p->res/2*p->zoom;
	int y = realFirst.y - p->res/2*p->zoom;
	int w = realLast.x-realFirst.x;
	int h = realLast.y-realFirst.y;

	SDL_Rect terrainRect = { x, y, w, h };//{ cameraPos.x, cameraPos.y, size.x, size.y };
	SDL_RenderCopy(p->renderer, p->geographyTexture, NULL, &terrainRect);

	for (const tile::id_type& id : tile::group_order)
	{
		auto& group = data.find(id);
		if (group != end(data))
		{
			auto& groupKey = group->first;
			//for (auto& stack : group->second)
			auto screen = p->screenSize * p->res / p->zoom * 0.1;
			auto offset = p->Transform(p->offset, p->SCREENSPACE, p->SIMSPACE) * 0;
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
							int index = (std::hash<std::string>()(f->region)+std::hash<int>()(f->regionID)) % p->fireColors->w;
							SDL_LockSurface(p->fireColors);
							Uint8* pixel = reinterpret_cast<Uint8*>(reinterpret_cast<Uint32*>(p->fireColors->pixels) + index);
							Uint8 r = *(pixel+0);
							Uint8 g = *(pixel+1);
							Uint8 b = *(pixel+2);
							Uint8 a = *(pixel+3);
							SDL_UnlockSurface(p->fireColors);
							SDL_Color c = SDL_Color{r,g,b,a};

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

								auto asset = p->find(0xF1);
								SDL_SetTextureColorMod((*asset)->second.getTexture(), c.r, c.g, c.b);
								p->draw(asset, pos);
								//highlightCell(pos, c);
							}
						}

						if (groupKey == tile::OBJECTGROUP) {
							if (item->burnable == false)
								fillCell(pos, SDL_Color{ 255, 128, 0, 128 });
							//auto& id = item->id;
							//draw(id, pos);
						}
						if (groupKey == tile::UNITGROUP) {
							tile::Unit* f = reinterpret_cast<tile::Unit*>(item);
							if (f->destination.is_initialized())
								highlightCell(f->destination.get(), SDL_Color{ 255, 0, 0, 128 });	
							auto& id = item->id;
							draw(id, pos);
						}

						/*if (groupKey == tile::GEOGRAPHYGROUP) {
							tile::Land* f = reinterpret_cast<tile::Land*>(item);
							int h = f->elevation;
							h *= 4;
							if (h < 0) h = 0;
							if (h > 255) h = 255;

							if (f->isWater())
							fillCell(pos, SDL_Color{ 0, 0, 255, 64 });
							else {
							p->bufferTerrain(pos.x, pos.y, h / 255.0);
							scalar realPos = p->Transform(pos, p->SIMSPACE, p->CAMERASPACE);
							if (static_cast<int>(floor(pos.x)) % 4 == 0 && static_cast<int>(floor(pos.y)) % 4 == 0) {
							int w = p->res*p->zoom * 4;
							SDL_Rect r = SDL_Rect{ realPos.x - w, realPos.y - w, w * 3, w * 3 };
							SDL_SetTextureBlendMode(p->tileBuffer, SDL_BLENDMODE_BLEND);
							SDL_SetTextureAlphaMod(p->tileBuffer, 128);
							Render(p->renderer, p->tileBuffer, NULL, &r);
							}
							}*/
						//}
						//	else if (h < 200) {
						//		fillCell(pos, SDL_Color{ h*1.8, 64 - h, h*1.2, 64 - h / 4 });
						//	}
						//	else {
						//		fillCell(pos, SDL_Color{ 255, 255, 255, 64 + (h - 200) * 5 });
						//	}
						//}
						//if (groupKey == tile::OBJECTGROUP) {
						//	fillCell(pos, SDL_Color{ 0, 255, 0, 5 });
						//}
						//if (groupKey == tile::UNITGROUP) {
						//	auto& id = item->id;
						//	draw(id, pos);
						//}

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
	if (query.is_initialized())
		p->draw(query, pos);
}

void CLASS::clear()
{
	SDL_SetRenderDrawColor(p->renderer, 65, 60, 60, 255);
	SDL_RenderClear(p->renderer);
}

void CLASS::loadAsset(const std::string& path, int key)
{
	p->loadAsset(path, key);
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

/*Dont' call twice unless you free p->geographytexture*/
void CLASS::drawTerrain(facet::master_group_type* terrain, facet::master_group_type* objects) {
	//SDL_DestroyTexture(p->geographyTexture);
	p->geographyTexture = p->createTerrainTexture(terrain,objects);
}