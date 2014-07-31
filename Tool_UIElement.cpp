#include "stdafx.h"
#include "scalar.h"
#include "Tool_UIElement.h"
#include <boost\property_tree\ptree.hpp>
#include <SDL2\SDL.h>
#include <SDL2\SDL_ttf.h>
#include <SDL2\SDL_image.h>
#include "ArtHelp.h"
#include <sstream>

/*------------------------------------------------------------------------------------------------------------*
												HELPER CODE
*-------------------------------------------------------------------------------------------------------------*/
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