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
			window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, size.x, size.y, windowFlags);
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
			rect = SDL_Rect{ offset.x, offset.y, size.x, size.y };
		}
		Area(int x, int y, int w, int h) {
			rect = SDL_Rect{ x, y, w, h };
		}
	};


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
		Texture() = delete;
		Texture(const Context* ctx, const std::string& path) : data{ makeTexture(ctx, path) } {

		}
		Texture(Texture&& other) : data{ std::move(other.data) }, context{ std::move(other.context) } {}
		Texture(std::unique_ptr<Context>& ctx, const std::string& path) :
			data{ makeTexture(ctx.get(), path) }
		{
			context = ctx.get();
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
			result.rect.x += transform.position.x;
			result.rect.y += transform.position.y;
			result.rect.w += result.rect.w * transform.scale.x;
			result.rect.h += result.rect.h * transform.scale.y;
			result.rect.x -= (result.rect.w - area.rect.w) / 2;
			result.rect.y -= (result.rect.h - area.rect.h) / 2;
			return result;
		}
		int draw() {
			return SDL_RenderCopy(**texture->context, **texture, NULL, *applyTransform());
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
	void default_delete<SDL_Texture>::operator()(SDL_Texture* _ptr) const { SDL_DestroyTexture(_ptr); }
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
		return SDL_CreateTexture(artist, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size.x, size.y);
	}
	UI::art::area area(const scalar& size, const scalar& position = scalar(0, 0)) {
		return UI::art::area{ position.x, position.y, size.x, size.y };
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

		for (int y = position.y - radius; y <= position.y + radius; y++)
			for (int x = position.x - radius; x <= position.x + radius; x++)
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
	area = UI::art::area{ a.get<int>("x"), a.get<int>("y"), a.get<int>("w"), a.get<int>("h") };
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
	//Texture staticData;
	Interface() = default;
	virtual UI::Image* renderBackground() = 0;
	virtual void update(UI::info* info = NULL) = 0;
	virtual int draw(UI::Image image) = 0;
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
	virtual UI::Image* renderBackground() {
		auto background = new Image(context, info.get_child("Area"));
		area = background->area;
		auto color = info.get_optional<std::string>("Background.Color");
		auto imagePath = info.get_optional<std::string>("Background.Image");
		auto renderer = get::artist(context);
		SDL_SetRenderTarget(renderer, background->canvas);

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
	virtual int draw(UI::Image image)
	{
		return render::image(&image);
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
	virtual int draw(UI::Image image)
	{
		BasicImplementation::draw(image);
		auto center = SDL_Point{ area.w*.5, area.h*.5 };
		SDL_RenderCopyEx(context.second, needle, NULL, &area, rotation * (180/3.14), &center, SDL_FLIP_NONE);
		return 1;
	}
};

class UI::SelectionUI : public UI::Interface{
public:
	using Texture = modern::Texture;
	using Image = modern::StaticImage;
	using Context = modern::Context;
	using Config = modern::Config;
	using Area = modern::Area;
	using Transform = modern::Transform;

	using string = std::string;

	using Item = std::pair<Texture*, Image*>;
	using Container = std::vector<Item>;

	//
	const int size;
	Context* context;
	Container icons;

	SelectionUI(SDL_Window* w, SDL_Renderer* r, const int& iconSize, std::vector<string> items) : size{ iconSize } {
		icons = Container();
		*context = Context{ w, r };
		for (const string& path : items) {
			Texture* t = new Texture(context, path);
			Image* i = new Image(
				t,
				Area(scalar(iconSize, iconSize)),
				Transform(scalar(0, 0), scalar(1, 1), 0)
				);
			icons.push_back(std::make_pair(t, i));
		}
	}

	/*Requires cfg contains an integer called iconsize at its root.*/
	SelectionUI(UI::art::context* ctx, UI::info* cfg) : size{ cfg->get<int>("iconSize") }
	{
		context = new Context(ctx->first, ctx->second);
		for (auto& item : cfg->get_child("Items"))
		{
			Texture* t = new Texture(context, item.second.get<std::string>("Icon"));
			//data
		}
	}

	virtual UI::Image* renderBackground() {
		return nullptr;
	}

	virtual void update(UI::info* info = NULL) {
		int i = 0;
		for (auto& item : icons)
		{
			int desiredX = i*size;
			++i;
		}
	}

	virtual int draw(UI::Image image) {
		for (int i = 0; i < icons.size(); i++)
			icons[i].second->draw();
		return 1;
	}

	virtual ~SelectionUI() {
		for (int i = 0; i < icons.size(); i++){
			delete icons[i].first;
			delete icons[i].second;
		}
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
		return new SelectionUI(&ctx, &cfg);
	}
	else {
		return new BasicImplementation(ctx, &cfg);
	}
}



UI::UI(UI::art::context& ctx, UI::info& cfg) : detail{ makeDetail(cfg,ctx) }
{
	background = detail->renderBackground();
}
UI::~UI(){}
void UI::draw()
{
	detail->draw(*background);
}
void UI::update(UI::info* newData)
{
	detail->update(newData);
}