#include "stdafx.h"
#include "scalar.h"
#include "Tool_UIElement.h"
#include <boost\property_tree\ptree.hpp>
#include <SDL2\SDL.h>
#include <SDL2\SDL_ttf.h>
#include <SDL2\SDL_image.h>
#include "ArtHelp.h"
#include <boost\property_tree\ptree.hpp>
#include <boost\property_tree\info_parser.hpp>
#include <sstream>
#include <array>


/*------------------------------------------------------------------------------------------------------------*
												HELPER CODE
*-------------------------------------------------------------------------------------------------------------*/
namespace std {
	void default_delete<SDL_Texture>::operator()(SDL_Texture* _ptr) const { SDL_DestroyTexture(_ptr); }
}

namespace modern {

	struct Context {
		SDL_Window* window;
		SDL_Renderer* renderer;
		SDL_Window* getWindow() const { return window; }
		SDL_Renderer* getRenderer() const { return renderer; }
		SDL_Renderer* operator*() const { return getRenderer(); }
		Context() = delete;
		Context(SDL_Window* w, SDL_Renderer* r) { window = w; renderer = r; }
		Context(Context&& other) : window{ std::move(other.window) }, renderer{ std::move(other.renderer) } {}
		Context(const scalar& size, Uint32 windowFlags = 0, Uint32 renderFlags = SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_ACCELERATED) {
			window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, static_cast<int>(size.x), static_cast<int>(size.y), windowFlags);
			renderer = SDL_CreateRenderer(window, -1, renderFlags);
		}
	};

	struct Area {
		SDL_Rect rect;
		bool empty = false;
		SDL_Rect* operator*() {
			if (empty)
				return NULL;
			return &rect;
		}
		Area() {
			rect = SDL_Rect{ 0, 0, 0, 0 };
			empty = true;
		}
		Area(scalar size) {
			scalar offset = size / -2;
			rect = SDL_Rect{ static_cast<int>(offset.x), static_cast<int>(offset.y), static_cast<int>(size.x), static_cast<int>(size.y) };
		}
		Area(int x, int y, int w, int h) {
			rect = SDL_Rect{ x, y, w, h };
		}
		bool Area::contains(int x, int y)
		{
			return (x > rect.x && x < rect.x + rect.w && y > rect.y && y < rect.y + rect.w);
		}
	};

	/*Wrapper around SDL_Texture, allowing for textures to be automatically cleaned up, and be bound to their window/renderer.*/
	struct Texture {
		std::unique_ptr<SDL_Texture> data;
		const Context* context;
		SDL_Texture* operator*() const { return data.get(); }
		SDL_Texture* makeTexture(const Context* context, const std::string& path) {
			auto temp = IMG_Load(path.c_str());
			auto ptr = SDL_CreateTextureFromSurface(**context, temp);
			SDL_FreeSurface(temp);
			return ptr;
		}
		Texture(){}
		Texture(const Context* ctx, const std::string& path) : data{ makeTexture(ctx, path) } {
			context = ctx;
		}
		Texture(const Context* ctx, SDL_Texture* t) : data{ t } {
			context = ctx;
		}
		Texture(Texture&& other) : data{ std::move(other.data) }, context{ std::move(other.context) } {}
		Texture(std::unique_ptr<Context>& ctx, const std::string& path) :
			data{ makeTexture(ctx.get(), path) }
		{
			context = ctx.get();
		}
		Area getRect(int x = 0, int y = 0) {
			Area area = Area{ x, y, 16, 16 };
			SDL_QueryTexture(data.get(), NULL, NULL, &area.rect.w, &area.rect.h);
			return area;
		}
		int draw(Area& dst = Area(), Area& src = Area()) {
			return SDL_RenderCopy(**context, data.get(), *src, *dst);
		}
	};

	struct Transform {
		scalar position;
		scalar scale;
		double rotation;
		Transform() {
			position = scalar{};
			scale = scalar{};
			rotation = 0;
		}
		Transform(scalar _position, scalar _scale = scalar{ 0 }, double _rotation = 0) {
			position = _position;
			scale = _scale;
			rotation = _rotation;
		}
	};

	struct Config {
		using ptree = boost::property_tree::ptree;
		ptree data;
		ptree* operator*(){ return &data; }
		Config(const std::string& path) { boost::property_tree::read_info(path, data); }
		Config(const ptree& input) { data = input; }
	};

	struct StaticImage {
		/*texture points to a Texture. One Texture per image. Texture becomes invalid at the end of its scope(unique_ptr)
		Can change the pointer, but not the texture.*/
		const Texture*	texture;
		Area		area;
		Transform	transform;

		const Context* context() {
			return texture->context;
		}
		StaticImage() {
			texture = NULL;
			area = Area();
			transform = Transform();
		}
		StaticImage(Texture* image, Area a, Transform&& t = Transform()) {
			texture = image;
			area = a;
			transform = t;
		}
		Area applyTransform() {
			Area result = area;
			result.rect.x += static_cast<int>(transform.position.x);
			result.rect.y += static_cast<int>(transform.position.y);
			result.rect.w += static_cast<int>(result.rect.w * transform.scale.x);
			result.rect.h += static_cast<int>(result.rect.h * transform.scale.y);
			result.rect.x -= static_cast<int>((result.rect.w - area.rect.w) / 2);
			result.rect.y -= static_cast<int>((result.rect.h - area.rect.h) / 2);
			return result;
		}
		int draw() {
			return SDL_RenderCopy(texture->context->getRenderer(), texture->data.get(), NULL, *applyTransform());
		}
	};

	struct Text : public Texture {
		std::string content;
		SDL_Color color;
		int x;
		int y;
		Text(Context* context, std::string text, int px, int py, TTF_Font* font, SDL_Color c = SDL_Color{ 255, 255, 255, 255 }) : Texture(context, makeText(context,text,font))
		{
			x = px;
			y = py;
			content = text;
			color = c;
		}
		SDL_Texture* makeText(Context* context, std::string text, TTF_Font* font)
		{
			SDL_Surface* temp = TTF_RenderText_Blended(font, text.c_str(), color);
			SDL_Texture* tex = SDL_CreateTextureFromSurface(context->getRenderer(), temp);
			SDL_FreeSurface(temp);
			return tex;
		}
		int draw() {
			return Texture::draw(getRect(x,y));
		}
	};

	struct Button {
		Context* ctx;
		TTF_Font* font;
		SDL_Colour backgroundColour;
		Area area;
		bool hovering;
		boost::optional<StaticImage> backgroundImage;
		std::vector<Text*> texts;
		Button(Context* context, Area a, SDL_Color bg = SDL_Color{ 0, 0, 0, 0 }, TTF_Font* f = NULL){ ctx = context; area = a; font = f; backgroundColour = bg; }
		void addText(int x, int y, std::string content, SDL_Color color = SDL_Color{ 255, 255, 255, 255 }) {
			if (font!=NULL)
				texts.push_back(new Text(ctx, content, x, y, font, color));
		}
		void update(int mx, int my) {
			hovering = area.contains(mx, my);
		}
		int draw() {
			SDL_SetRenderDrawColor(ctx->renderer, backgroundColour.r, backgroundColour.g, backgroundColour.b, backgroundColour.a);
			SDL_RenderFillRect(ctx->renderer, *area);
			if (backgroundImage.is_initialized()) {
				backgroundImage->draw();
			}
			if (hovering) {
				SDL_SetRenderDrawColor(ctx->renderer, 60, 60, 60, 60);
				SDL_RenderFillRect(ctx->renderer, *area);
			}
			for (auto& text : texts)
			{
				text->draw();
			}
			return 1;
		}
		~Button() {
			for (auto ptr : texts)
			{
				delete ptr;
			}
		}
	};

	template <int Tilesize>
	struct Tile {
		StaticImage image;
		bool empty = true;
		Tile(int x = 0, int y = 0, Texture* t = NULL)
		{
			if (t != NULL)
			{
				empty = false;
				image = StaticImage(t, Area(x*Tilesize, y*Tilesize, Tilesize, Tilesize), Transform());
			}
		}
		void draw()
		{
			if (!empty)
				image.draw();
		}
	};
}
namespace std {
	/*The cleanup for @SDL_Texture is defined here for unique_ptrs. */
//	void default_delete<SDL_Texture>::operator()(SDL_Texture* _ptr) const { SDL_DestroyTexture(_ptr); }
	/*The cleanup for @Context is defined here for unique_ptrs. */
	void default_delete<modern::Context>::operator()(modern::Context* _ptr) const {
		SDL_DestroyRenderer(_ptr->renderer);
		SDL_DestroyWindow(_ptr->window);
	}
}

namespace get {
	using boost::property_tree::ptree;
	UI::art::frame		frame	(const UI::art::context& ctx)	{  return ctx.first;  }
	UI::art::frame		window	(const UI::art::context& ctx)	{  return ctx.first;  } // alias
	UI::art::artist		artist	(const UI::art::context& ctx)	{  return ctx.second;  }
	UI::art::artist		renderer(const UI::art::context& ctx)	{  return ctx.second;  } // alias
	scalar				size	(UI::art::area area)			{  return scalar(area.w, area.h);  }
	scalar				position(UI::art::area area)			{  return scalar(area.x, area.y);  }
}
namespace make {
	UI::art::canvas canvas(UI::art::artist artist, const scalar& size) {
		return SDL_CreateTexture(artist, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, static_cast<int>(size.x), static_cast<int>(size.y));
	}
	UI::art::area area(const scalar& size, const scalar& position = scalar(0, 0)) {
		return UI::art::area{ static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(size.x), static_cast<int>(size.y) };
	}
	void target(UI::art::artist a, UI::art::canvas c) {
		SDL_SetRenderTarget(a, c);
	}
}
namespace render {
	int image(UI::Image* image, UI::art::area* frame = NULL) {
		return SDL_RenderCopy(get::artist(image->context), image->canvas, frame, &image->area);
	}
	int circle(const UI::art::context& context, scalar const&position, double const&radius) {
		
		std::vector<SDL_Point> points;

		for (int y = static_cast<int>(position.y - radius); y <= static_cast<int>(position.y + radius); y++)
			for (int x = static_cast<int>(position.x - radius); x <= static_cast<int>(position.x + radius); x++)
				points.push_back(SDL_Point{ x, y });

		return SDL_RenderDrawPoints(get::renderer(context), points.data(), points.size());
	}
}

UI::Image::Image(UI::art::context p_context, int const& w, int const& h)
: context{ p_context }, area(make::area(scalar(w, h))), canvas{ make::canvas(get::artist(p_context), get::size(area)) }
{
}

UI::Image::Image(UI::art::context p_context, const UI::info& a)
{
	context.first = p_context.first;
	context.second = p_context.second;
	int x, y, w, h;
	int screenw, screenh;
	SDL_GetWindowSize(p_context.first, &screenw, &screenh);

	auto offset = [](int& val, int& relto, UI::info& item) {
		auto _data = item.data();
		bool relative = _data[_data.size() - 1] == '%';
		double value = relative ? std::stod(_data.substr(0, _data.size() - 1)) : std::stod(_data);
		if (relative)
			value = value* relative / 100.0;
		val += value;
	};

	for (auto i : a )
	{
		if (i.first == "x")
			offset(x, screenw, i.second);
		if (i.first == "y")
			offset(y, screenh, i.second);
		if (i.first == "w")
			offset(w, screenw, i.second);
		if (i.first == "h")
			offset(h, screenh, i.second);
	}

	area = UI::art::area{ x,y,w,h };
	canvas = make::canvas(get::artist(p_context), get::size(area));
}

SDL_Point UI::Image::center()
{
	return SDL_Point{ area.w / 2, area.h / 2 };
}

UI::Image::Image() : context{ NULL, NULL }, canvas{ NULL }
{
	area = { 0, 0, 0, 0 };
}

/*------------------------------------------------------------------------------------------------------------*
*-------------------------------------------------------------------------------------------------------------*/

class UI::Interface
{
public:
	bool alive = true;
	//Texture staticData;
	UI::Image* background = nullptr;
	Interface() = default;
	virtual UI::Image* renderBackground() = 0;
	virtual void update(UI::info* info = NULL) = 0;
	virtual UI::info&& getRequestStructure() = 0;
	virtual int draw() = 0;
	virtual ~Interface() = default;
};

class UI::BasicImplementation : public UI::Interface
{
public:
	UI::info info;
	UI::art::context context;
	UI::art::area area;
	
	BasicImplementation(const UI::art::context ctx, UI::info* config)
	{
		info = UI::info(*config);
		context = ctx;
	}
	virtual ~BasicImplementation()
	{
	}

	virtual UI::info&& getRequestStructure() { return std::move(UI::info{}); }
	virtual UI::Image* renderBackground() {

		if (background == nullptr)
			background = new Image(context, info.get_child("Area"));

		area = background->area;
		auto color = info.get_optional<std::string>("Background.Color");
		auto imagePath = info.get_optional<std::string>("Background.Image");
		auto renderer = get::artist(context);
		SDL_SetRenderTarget(renderer, background->canvas);

		SDL_RenderClear(renderer);

		if (color.is_initialized()) {
			SDL_Color c{};
			std::istringstream data{ color.get().c_str() };
			data >> c.r >> c.g >> c.b >> c.a;
			SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
			SDL_RenderClear(renderer);
		}

		if (imagePath.is_initialized()) {
			auto surf = mem::wrap<gfx::surface>()(IMG_Load(imagePath->c_str()));
			auto tex = mem::wrap<gfx::canvas>()(SDL_CreateTextureFromSurface(renderer, surf.get()));
			SDL_RenderCopy(renderer, tex.get(), NULL, NULL);
		}
		//SDL_RenderPresent(renderer);
		SDL_SetRenderTarget(renderer, NULL);
		return background;
	}
	virtual void update(UI::info* p_info = NULL)
	{
	}
	virtual int draw()
	{
		return render::image(background);
	}
};

class UI::CompassUI : public UI::BasicImplementation {
public:
	SDL_Texture* needle;
	double rotation;
	CompassUI(const UI::art::context ctx, UI::info* config) : BasicImplementation(ctx, config)
	{
		auto surf = IMG_Load(config->get<std::string>("Background.Needle").c_str());
		needle = SDL_CreateTextureFromSurface(get::renderer(ctx), surf);
		SDL_FreeSurface(surf);
	}
	~CompassUI()
	{
		SDL_DestroyTexture(needle);
	}

	virtual UI::info&& getRequestStructure() { return std::move(UI::info{}); }
	virtual void update(UI::info* p_info = NULL)
	{
		if (p_info != NULL)
		{
			const auto& needleInfo = p_info->get_child_optional("Wind");
			if (needleInfo.is_initialized())
			{
				scalar wind = scalar(
					needleInfo->get<double>("N") - needleInfo->get<double>("S"),
					needleInfo->get<double>("E") - needleInfo->get<double>("W")
				);
				rotation = atan2(wind.y, wind.x);
			}
		}
	}
	virtual int draw()
	{
		BasicImplementation::draw();
		auto center = SDL_Point{ static_cast<int>(area.w*.5), static_cast<int>(area.h*.5) };
		SDL_RenderCopyEx(context.second, needle, NULL, &area, rotation * (180/3.14), &center, SDL_FLIP_NONE);
		return 1;
	}
};

class UI::ListUI : public UI::Interface {
public:
	using Texture = modern::Texture;
	using MenuItem = modern::Button;
	using Context = modern::Context;
	using Config = modern::Config;
	using Area = modern::Area;
	using Transform = modern::Transform;

	using string = std::string;

	using Item = std::pair<Texture*, MenuItem*>;
	using Container = std::vector<Item>;

	//
	double expansion = 0.0;
	const int size;
	Context* context;
	Container icons;
	TTF_Font* font;
	Area area;
	int incidents;
	int acknowledgedIncidents = 0;
	ListUI(SDL_Window* w, SDL_Renderer* r, const int& iconSize, std::vector<string> items) : size{ iconSize } {
		init();
		*context = Context{ w, r };
		for (const string& path : items) {
			Texture* t = new Texture(context, path);
			MenuItem* i = new MenuItem(context, Area{ 0, 0, size, size }, SDL_Color{ 60, 60, 60, 60 }, font);
			icons.push_back(std::make_pair(t, i));
		}
	}

	/*Requires cfg contains an integer called iconsize at its root.*/
	ListUI(UI::art::context* ctx, UI::info* cfg) : size{ cfg->get<int>("iconSize") }
	{
		init();
		context = new Context(ctx->first, ctx->second);

		auto a = cfg->get_child("Area");
		int x, y, w, h;
		x = a.get<int>("x");
		y = a.get<int>("y");
		w = a.get<int>("w");
		h = a.get<int>("h");

		if (a.data() == "relative")
		{
			int screenW = 0; int screenH = 0;
			SDL_GetWindowSize(ctx->first, &screenW, &screenH);
			x = screenW * x / 100;
			y = screenH * y / 100;
			w = screenW * w / 100;
			h = screenH * h / 100;
		}

		area = Area(x, y, w, h);

		int offset = 0;

		std::vector<std::string> texts = { { "Cancel", "Move", "FireBreak", "FireFight" } };
		using boost::property_tree::ptree;
		auto items = cfg->get_child_optional("Items");
		if (items.is_initialized())
		{
			for (auto& item : items.get())
			{
				Texture* t = new Texture(context, item.second.get<std::string>("Icon"));
				MenuItem* i = new MenuItem(context, Area(x + (offset * 10) + (w*++offset), y, w, h), SDL_Color{ 255, 0, 0, 255 }, font);
				i->addText(i->area.rect.x + 10, i->area.rect.y + 10, std::to_string(offset));
				i->addText(i->area.rect.x + 10, i->area.rect.y + 40, texts[offset - 1]);
				icons.push_back(std::make_pair(t, i));
				//data
			}
		}
	}

	virtual ~ListUI() {
		for (unsigned int i = 0; i < icons.size(); i++){
			delete icons[i].first;
			delete icons[i].second;
		}
		TTF_CloseFont(font);
	}
	virtual UI::info&& getRequestStructure() {
		std::istringstream request("Incidents\n");
		UI::info result;
		boost::property_tree::read_info(request, result);
		return std::move(result);
	}

	void init() {
		expansion = 0;
		font = TTF_OpenFont("normalFont.ttf", 32);
		alive = true;
		icons = Container();
	}

	virtual UI::Image* renderBackground() {
		return nullptr;
	}

	virtual void update(UI::info* info = NULL) {
		int i = 0;
		if (!alive)
			return;

		auto mouseData = info->get_child_optional("Mouse");
		int x, y, left, right, middle; //mouse info
		bool mouseUpdated = mouseData.is_initialized();
		if (mouseUpdated)
		{
			x = mouseData->get<int>("x");
			y = mouseData->get<int>("y");
			left = mouseData->get_optional<int>("left").get_value_or(0);
			right = mouseData->get_optional<int>("right").get_value_or(0);
			middle = mouseData->get_optional<int>("middle").get_value_or(0);
		}
		auto incidentUpdate = info->get_optional<int>("Incidents");
		if (incidentUpdate.is_initialized())
		{
			incidents = incidentUpdate.get();
		}
		if (area.contains(x, y) && left > 0)
		{
			acknowledgedIncidents = incidents;
		}
	}

	virtual int draw() {
		modern::Text&& text = modern::Text(context, std::to_string(incidents-acknowledgedIncidents)+" Incidents", area.rect.x + 10, area.rect.y + 10, font, SDL_Color{ 0, 0, 0, 255 });
		SDL_SetRenderDrawColor(context->getRenderer(), 255, 255, 255, 255);
		SDL_RenderFillRect(context->getRenderer(), &area.rect);
		text.draw();
		return 1;
	}
};

class UI::MenuUI : public UI::Interface {
public:
	using Texture = modern::Texture;
	using MenuItem = modern::Button;
	using Context = modern::Context;
	using Config = modern::Config;
	using Area = modern::Area;
	using Transform = modern::Transform;

	using string = std::string;

	using Item = std::pair<Texture*, MenuItem*>;
	using Container = std::vector<Item>;

	//
	double expansion = 0.0;
	const int size;
	Context* context;
	Container icons;
	TTF_Font* font;

	MenuUI(SDL_Window* w, SDL_Renderer* r, const int& iconSize, std::vector<string> items) : size{ iconSize } {
		init();
		*context = Context{ w, r };
		for (const string& path : items) {
			Texture* t = new Texture(context, path);
			MenuItem* i = new MenuItem(context, Area{ 0, 0, size, size }, SDL_Color{ 60, 60, 60, 60 }, font);
			icons.push_back(std::make_pair(t, i));
		}
	}

	/*Requires cfg contains an integer called iconsize at its root.*/
	MenuUI(UI::art::context* ctx, UI::info* cfg) : size{ cfg->get<int>("iconSize") }
	{
		init();
		context = new Context(ctx->first, ctx->second);

		auto a = cfg->get_child("Background.Area");
		int x, y, w, h;
		x = a.get<int>("x");
		y = a.get<int>("y");
		w = a.get<int>("w");
		h = a.get<int>("h");

		int offset = 0;

		std::vector<std::string> texts = { { "Cancel", "Move", "FireBreak", "FireFight" } };

		//for (auto& item : cfg->get_child("Items"))
		for (int n = 0; n < 4; n++)
		{
			Texture* t = new Texture();//(context, item.second.get<std::string>("Icon"));
			MenuItem* i = new MenuItem(context, Area(x + (offset*10) + (w*++offset), y, w, h), SDL_Color{ 255, 0, 0, 255 }, font);
			i->addText(i->area.rect.x + 10, i->area.rect.y + 10, std::to_string(offset));
			i->addText(i->area.rect.x + 10, i->area.rect.y + 40, texts[offset - 1]);
			icons.push_back(std::make_pair(t, i));
			//data
		}
	}

	virtual UI::info&& getRequestStructure() { return std::move(UI::info{}); }


	void init() {
		expansion = 0;
		font = TTF_OpenFont("normalFont.ttf", 12);
		alive = true;
		icons = Container();
	}

	virtual UI::Image* renderBackground() {
		return nullptr;
	}

	virtual void update(UI::info* info = NULL) {
		int i = 0;
		if (!alive)
			return;

		auto mouseData = info->get_child_optional("Mouse");
		int x, y, left, right, middle; //mouse info
		bool mouseUpdated = mouseData.is_initialized();
		if (mouseUpdated)
		{
			x = mouseData->get<int>("x");
			y = mouseData->get<int>("y");
			left = mouseData->get_optional<int>("left").get_value_or(0);
			right = mouseData->get_optional<int>("right").get_value_or(0);
			middle = mouseData->get_optional<int>("middle").get_value_or(0);
		}

		for (auto icon : icons)
		{
			icon.second->update(x, y);
			if (icon.second->hovering && left == 2)
				alive = false;
		}

		expansion += (1 - expansion)*0.1;
	}

	virtual int draw() {
		for (unsigned int i = 0; i < icons.size(); i++)
			icons[i].second->draw();
		return 1;
	}

	virtual ~MenuUI() {
		for (unsigned int i = 0; i < icons.size(); i++){
			delete icons[i].first;
			delete icons[i].second;
		}
		TTF_CloseFont(font);
	}
};

UI::Interface* UI::makeDetail(UI::info& cfg, UI::art::context& ctx)
{
	auto type = cfg.get<std::string>("Type");
	if (type=="Normal")
	{
		return new BasicImplementation(ctx, &cfg);
	}
	else if (type == "Compass")
	{
		return new CompassUI(ctx, &cfg);
	}
	else if (type == "Menu")
	{
		return new MenuUI(&ctx, &cfg);
	}
	else if (type == "List")
	{
		return new ListUI(&ctx, &cfg);
	}
	else {
		return new BasicImplementation(ctx, &cfg);
	}
}



UI::UI(UI::art::context& ctx, UI::info& cfg) : detail{ makeDetail(cfg,ctx) }
{
	type = cfg.get<std::string>("Type");
	detail->renderBackground();
}
UI::~UI(){ type = "empty"; }
void UI::draw()
{
	detail->draw();
}
void UI::update(UI::info* newData)
{
	detail->update(newData);
}
bool UI::isAlive() {
	return detail->alive;
}
void UI::isAlive(bool setting) {
	detail->alive = setting;
}