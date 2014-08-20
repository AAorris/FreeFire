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

#include "ExperimentalGFX.h"

#define signs :

namespace Zen {


	struct AreaContract {
		virtual SDL_Rect* getArea() = 0;
		SDL_Rect output;

		bool contains(const int& px, const int& py) {
			SDL_Rect* rect = getArea();
			return rect->x < px && px < rect->x + rect->w
				&& rect->y < py && py < rect->y + rect->h;
		}
		template<typename _Composite>
		bool contains(const _Composite& pos) const { return contains(pos.x, pos.y); }

		virtual bool consume(const int& px, const int& py) {
			return contains(px, py);
		}

	}; using AreaSigned = AreaContract;

	struct RenderContext {

	};

	struct RenderPackage {
		//Take a number from the render context;
		int number;
		SDL_Color* fg = nullptr;
		SDL_Color* bg = nullptr;
	};

	enum axis { X, Y, Z, W,		A, B, C, D,		E, F, G, H,		I, J, K, L };


	template<typename T>
	struct Area signs AreaContract
	{
		using type = T;
		template <typename T> static type cast(T t) { return static_cast<type>(t); }
		type x, y, w, h;

		Area() : x{ 0 }, y{ 0 }, w{ 0 }, h{ 0 } {}

		Area(double p_x, double p_y, double p_w, double p_h) :
		x{ cast<type>(p_x) },
		y{ cast<type>(p_y) },
		w{ cast<type>(p_w) },
		h{ cast<type>(p_h) } {}

		template<typename base>
		Area(const Area<base>& a) : Area(a.x, a.y, a.w, a.h) {}

		type left() const { return x; }
		type top() const { return y; }
		type right() const { return x + w; }
		type bottom() const { return y + h; }
		type center(axis axis = X) const { return (axis==X) ? cast(x + w / 2) : (axis==Y) ? cast(y + h / 2) : 0; }

		template<typename T>
		Area<type> operator*(const T& val) const { 
			return Area<type>{x*val, y*val, w*val, h*val};
		}

		template<typename T>
		Area<type> operator+(const Area<T>& val) const {
			return Area<type>(x+val.x, y+val.y, w+val.w, h+val.h);
		}
		SDL_Rect* getArea() {
			//output is stored internally from areacontract
			output = { x, y, w, h };
			return &output;
		}
	}; using Areaf = Area<double>;

	SDL_Rect transform(SDL_Rect* rect, scalar& offset, scalar& scale) {
		SDL_Rect out;
		out.x = rect->x += offset.x;
		out.y = rect->y + offset.y;
		out.w = rect->w * scale.x;
		out.h = rect->h * scale.y;
		return out;
	}

	//template<typename _Parent>
	struct RelativeArea signs AreaContract {
		//using parent_type = _Parent;
		using parent_ptr = SDL_Rect*;
		parent_ptr relative;
		scalar offset;
		scalar scale = scalar(1,1);
		RelativeArea() : relative{ nullptr } {}
		RelativeArea(parent_ptr to) : relative{ to } {}
		SDL_Rect* getArea() {
			//output is stored internally when you sign an area contract
			output = transform(relative, offset, scale);
			return &output;
		}
	};

	//template<typename T> void draw(T& t, SDL_Renderer* r) { t.draw(r); }
	SDL_Renderer* current_renderer;
	void setRenderer(SDL_Renderer* r) { current_renderer = r; }
	void setColor(SDL_Color c) { SDL_SetRenderDrawColor(current_renderer, c.r, c.g, c.b, c.a); }
	template<typename T>
	void drawArea(Area<T>& area) { SDL_RenderFillRect(current_renderer, area.getArea()); }
	void drawArea(SDL_Rect* rect) { SDL_RenderFillRect(current_renderer, rect); }

	template<typename T>
	struct interpolate {
		T* from;
		double speed = 0.05;
		interpolate(T* it, const double& change) : from{ it }, speed{ change } {}
		void operator()(const T& to) {
			*from = *from*(1 - speed) + to*(speed);
		}
		void operator=(interpolate<T>& other) {
			from = other.from;
			speed = other.speed;
		}
	};

	template<typename T>
	struct basic_consumer {
		T* const plate;
		basic_consumer(T* ptr) : plate{ ptr } {}
		bool operator()(const int& x, const int& y) const {
			return plate->contains(x, y);
		}
	};

	struct DynamicArea signs AreaContract {
		//Typedefs
		using container = std::unordered_map<std::string, Area<double>>;
		//Members
		container states;
		Area<double> current;
		std::string goal;
		interpolate<Area<double>> lerp;
		SDL_Color backgroundColor = SDL_Color{ 255, 255, 255, 255 };
		//Constructors
		DynamicArea() : lerp{ &current, 0.05 } { 
			addStateArea("default", { 1<<4, 1<<4, 1<<6, 1<<6 });
			goal = "default";
			fixState("default");
		}
		DynamicArea(Area<double> default) : DynamicArea() { setState("default", default); fixState("default"); }
		void operator=(DynamicArea& other) {
			states = other.states;
			current = other.current;
			goal = other.goal;
			lerp = other.lerp;
			backgroundColor = other.backgroundColor;
		}
		//Functions
		void				addStateArea(std::string key, Area<double> area) {
			states.insert(std::make_pair(key, area));
		}
		void				addState(std::string key, std::vector<double> list) {
			states.insert(std::make_pair(key, Area<double>(list[0], list[1], list[2], list[3])));
		}
		void				setState	(std::string key, Area<double> area) { states[key] = area; }
		Area<double>&		iteratorValue(std::string key) { return getState(key)->second; }
		void				fixState	(std::string key)	{ goal = (key);  current = iteratorValue(goal); }
		void				update		()					{ 
			current = current * (1 - 0.05) + getState(goal)->second * (0.05);
		}
		void				setTarget	(std::string key)	{ goal = key;			}
		bool				contains	(int x, int y)		{ return current.contains(x,y);	}
		virtual SDL_Rect*	getArea		()					{ return current.getArea();		}
		container::iterator getState	(std::string key)	{ return states.find(key);		}
		void				setGoal		(std::string key)	{ getState(key); }
		bool				consume		(int x, int y)		{
			if(!contains(x,y)) return false;
			if (goal == "default") setTarget("small");
			else setTarget("default");
			return true;
		}
		void				draw		(SDL_Renderer* r)	{
			Zen::setRenderer(r);
			Zen::setColor(backgroundColor);
			Zen::drawArea(current);
		}
	};

	struct AccordianArea : public DynamicArea {
		RelativeArea clickable;
		AccordianArea() : DynamicArea(), clickable{ getArea() } {
			clickable.relative = &current.output;
		}
		void operator=(AccordianArea& other) {
			DynamicArea::operator=(other);
			clickable = other.clickable;
			clickable.relative = &current.output;
		}
		AccordianArea(Area<double> default, Area<double> small) : DynamicArea(default), clickable{ getArea() } {
			addStateArea("small", small);
			setClickableArea(0, 0, 0.1, 1);
		}
		void setClickableArea(int fromX, int fromY, double scaleX, double scaleY) {
			clickable.offset.x = fromX;
			clickable.offset.y = fromY;
			clickable.scale.x = scaleX;
			clickable.scale.y = scaleY;
			clickable.relative = &current.output;
		}
		void draw(SDL_Renderer* r) {
			DynamicArea::draw(r);
			//draw clickable area
			Zen::setColor(SDL_Color{ 0, 0, 0, 128 });
			Zen::drawArea(clickable.getArea());
		}
		void update() { DynamicArea::update(); }
		bool contains(int x, int y) { return DynamicArea::contains(x, y); }
		bool consume(int x, int y) { 
			if (!contains(x, y)) return false;
			if (clickable.consume(x, y))
			{
				if (goal == "default") setTarget("small");
				else setTarget("default");
				return true;
			}
			return false;
		}
		virtual SDL_Rect* getArea() {
			return DynamicArea::getArea();
		}
	};
/*
	struct ExpandableArea signs AreaContract {
		interpolate<Area<double>> lerp;
		subArea<Area<double>> getClickableArea;
		Area<double> closed;
		Area<double> expanded;
		Area<double> current;
		double howExpanded = 0;
		double expandTarget = 0;
		double transition_ms = 1000.0;
		double expansion_rate = 0.05;
		ExpandableArea() {}
		ExpandableArea(Area<double> c, Area<double> o) :
			closed{ c },
			expanded{ o }
		{
			lerp = interpolate<Area<double>>();
			getClickableArea = subArea<Area<double>>(-1, -1, 0.1, 1.0);
		}
		void update(int ms) {
			howExpanded = howExpanded * (1 - expansion_rate) + expandTarget * expansion_rate;
			current = lerp(closed, expanded, howExpanded);
		}
		SDL_Rect* getArea() {
			return current.getArea();
		}
		bool contains(int x, int y) const {
			return current.contains(x, y);
		}
	};*/

























}


class UI::Interface
{
public:
	bool alive = true;
	//Texture staticData;
	UI::Image* background = nullptr;
	Interface() = default;
	virtual UI::Image* renderBackground() = 0;
	virtual void update(const UI::info* info = NULL) = 0;
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
	virtual void update(const UI::info* p_info = NULL)
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
	virtual void update(const UI::info* p_info = NULL)
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

class clickable_ui : public UI::Interface {

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
	const int size;
	Context* context;
	Container icons;
	TTF_Font* font;
	Area area;
	Zen::AccordianArea expander;
	int incidents;
	int acknowledgedIncidents = 0;
	bool wasClicked = false;
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
	ListUI(UI::art::context* ctx, UI::info* cfg) : size{ cfg->get<int>("iconSize") }, expander()
	{
		init();
		context = new Context(ctx->first, ctx->second);

		auto a = cfg->get_child("Area");
		double x, y, w, h;
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
		
		expander = Zen::AccordianArea({ x, y, w, h }, { x + w*.9, y, w, h });
		//expander.setState("default", { x, y, w, h });
		//expander.addState("small", { x+w*.9, y, w*0.1, h });

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
		font = TTF_OpenFont("normalFont.ttf", 32);
		alive = true;
		icons = Container();
	}

	virtual UI::Image* renderBackground() {
		return nullptr;
	}

	virtual void update(const UI::info* info = NULL) {
		int i = 0;
		if (!alive)
			return;

		expander.update();

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
		if (incidentUpdate.is_initialized()){
			incidents = incidentUpdate.get();
		}
		if (expander.contains(x, y) && left > 0)
			wasClicked = true;
		if (left == 0 && wasClicked){
			bool consumed = expander.consume(x, y);
			if (!consumed)
				acknowledgedIncidents = incidents;
			wasClicked = false;
		}
	}

	virtual int draw() {
		expander.draw(context->renderer);
		modern::Text&& text = modern::Text(
			context, std::to_string(incidents-acknowledgedIncidents)+" Incidents", 
			expander.current.left() + expander.current.w*0.2, expander.current.top() + 10, font, SDL_Color{ 0, 0, 0, 255 });
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


		std::vector<std::string> texts = { { "Cancel" } };
		auto abilities = cfg->get_child_optional("Abilities");
		if (abilities.is_initialized())
		{
			for (auto i = abilities.get().begin(); i != abilities.get().end(); i++) {
				std::string one = i->first;
				std::string two = i->second.data();
				auto three = i->second.front().first;
				auto four = i->second.front().second.data();
				texts.push_back(i->second.front().first.data());
			}
		}

		//for (auto& item : cfg->get_child("Items"))
		for (int n = 0; n < texts.size(); n++)
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

	virtual void update(const UI::info* info = NULL) {
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
void UI::update(const UI::info* newData)
{
	detail->update(newData);
}
bool UI::isAlive() {
	return detail->alive;
}
void UI::isAlive(bool setting) {
	detail->alive = setting;
}