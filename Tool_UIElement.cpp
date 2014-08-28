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
#include "Facet_Sim.h"
#include <sstream>
#include <array>
#include <set>
#include "ExperimentalGFX.h"
#include "Area.h"
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

class UI::Interface
{
public:
	bool alive = true;
	//Texture staticData;
	UI::Image* background = nullptr;
	Facet_Sim* sim = nullptr;
	Interface(Facet_Sim* sim_ref = nullptr);
	virtual UI::Image* renderBackground() = 0;
	virtual bool update(const UI::info* info = NULL) = 0;
	virtual UI::info&& getRequestStructure() = 0;
	virtual int draw() = 0;
	virtual ~Interface() = default;
};
UI::Interface::Interface(Facet_Sim* sim_ref) {
	sim = sim_ref;
}

class UIContract
{
public:
	bool alive;
	/*This is read by the UI*/
	static UI::info* simData;
	/*This is read by the application when the UI is finished.*/
	UI::info  exitData;
	virtual void init(const UI::art::context, UI::info* config) = 0;
	virtual int draw() = 0;
	virtual bool update(const UI::info* updateData = nullptr) = 0;
	virtual ~UIContract() = default;
};

//struct IconSelector signs public UIContract
//{
//	struct Icon {
//		int index;
//		std::string name;
//		Zen::DynamicArea area;
//		bool operator<(const Icon& right) { return index < right.index; }
//	};
//	std::vector<Icon> icons;
//	IconSelector();
//	virtual void init(const UI::art::context, UI::info* config);
//	virtual int draw();
//	virtual void update(const UI::info* updateData = nullptr);
//	virtual ~IconSelector();
//protected:
//	double expandTime;
//	void addIcon(const std::string& name);
//	void select(const std::string& name);
//};
//
//IconSelector::IconSelector() {}
//void IconSelector::init(const UI::art::context, UI::info* config) {
//	auto width = config->get<double>("IconWidth");
//	auto height = config->get<double>("IconHeight");
//	auto& icons = config->get_child("Icons");
//	int nIcons = icons.size();
//	for (auto it : icons) {
//		auto index = icons.size();
//		auto indexChange = index - nIcons / 2.0;
//		auto& name = it.first;
//		auto& data = it.second;
//		Icon icon{};
//		icon.index = index;
//		icon.name = name;
//		icon.area.setState("Start", { nIcons/2*width, 0, width/4, height/4 });
//		icon.area.setState("Finish", { indexChange*width, 0, width, height });
//		icon.area.fixState("Start");
//		icon.area.setGoal("Finish");
//	}
//}
//void IconSelector::update(const UI::info* updateData)
//{
//	auto mouseX = simData->get<int>("Mouse.x");
//	auto mouseY = simData->get<int>("Mouse.y");
//	auto mouseButton = simData->get<int>("Mouse");
//	for (auto& item : icons) {
//		auto& area = item.area;
//		area.update();
//		if (area.contains(mouseX, mouseY)) //hovering
//		{
//
//		}
//	}
//}

class UI::BasicImplementation : public UI::Interface
{
public:
	UI::info info;
	UI::art::context context;
	UI::art::area area;
	
	BasicImplementation(const UI::art::context ctx, UI::info* config)
	{
		info = UI::info(*config);
		auto a = info.get_child("Area");

		area.x = a.get<int>("x");
		area.y = a.get<int>("y");
		area.w = a.get<int>("w");
		area.h = a.get<int>("h");
		context = ctx;
	}
	virtual ~BasicImplementation()
	{
	}

	virtual UI::info&& getRequestStructure() { return std::move(UI::info{}); }
	virtual UI::Image* renderBackground() {

		if (background == nullptr)
			background = new Image(context, info.get_child("Area"));

		//area = background->area;
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
	virtual bool update(const UI::info* p_info = NULL)
	{
		return false;
	}
	virtual int draw()
	{
		return render::image(background);
	}
};

class UI::CompassUI : public UI::BasicImplementation {
public:
	SDL_Texture* needle;
	SDL_Texture* body;
	double rotation;
	CompassUI(const UI::art::context ctx, UI::info* config) : BasicImplementation(ctx, config)
	{
		auto surf = IMG_Load(config->get<std::string>("Background.Needle").c_str());
		auto bsurf = IMG_Load(config->get<std::string>("Background.Image").c_str());
		needle = SDL_CreateTextureFromSurface(get::renderer(ctx), surf);
		body = SDL_CreateTextureFromSurface(get::renderer(ctx), bsurf);
		SDL_FreeSurface(bsurf);
		SDL_FreeSurface(surf);
	}
	~CompassUI()
	{
		SDL_DestroyTexture(needle);
	}

	virtual UI::info&& getRequestStructure() { return std::move(UI::info{}); }
	virtual bool update(const UI::info* p_info = NULL)
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
		return false;
	}
	virtual int draw()
	{
		BasicImplementation::draw();
		auto center = SDL_Point{ static_cast<int>(area.w*.5), static_cast<int>(area.h*.5) };
		SDL_RenderCopy(get::renderer(context), body, NULL, &area);
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

	ListUI(SDL_Window* w, SDL_Renderer* r, const int& iconSize, std::vector<string> items, Facet_Sim* sim_ref) : Interface{ sim_ref }, size{ iconSize } {
		init();
		*context = Context{ w, r };
		for (const string& path : items) {
			Texture* t = new Texture(context, path);
			MenuItem* i = new MenuItem(context, Area{ 0, 0, size, size }, SDL_Color{ 60, 60, 60, 60 }, font);
			icons.push_back(std::make_pair(t, i));
		}
	}

	/*Requires cfg contains an integer called iconsize at its root.*/
	ListUI(UI::art::context* ctx, UI::info* cfg, Facet_Sim* sim_ref) : Interface{ sim_ref }, size{ cfg->get<int>("iconSize") }, expander()
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
		
		expander = Zen::AccordianArea({ x, y, w, h }, { x + w*.8, y, w, h });
		expander.setClickableArea(0, 0.4*h, 0.2, 0.2);
		expander.backgroundColor = {0,0,0,255*.7};
		//expander.setState("default", { x, y, w, h });
		//expander.addState("small", { x+w*.9, y, w*0.1, h });
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
		font = TTF_OpenFont("normalFont.ttf", 12);
		alive = true;
		icons = Container();
	}

	virtual UI::Image* renderBackground() {
		return nullptr;
	}

	virtual bool update(const UI::info* info = NULL) {
		int i = 0;
		if (!alive)
			return false;

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

		return wasClicked;
	}

	virtual int draw() {
		expander.draw(context->renderer);
		modern::Text&& text = modern::Text(
			context, std::to_string(incidents-acknowledgedIncidents)+" Incidents", 
			expander.current.left() + expander.current.w*0.3, expander.current.top() + 10, font, SDL_Color{ 255, 255, 255, 255 });
		text.draw();

		int offset = expander.current.top() + 60;

		if (sim != nullptr) {
			int x, y;
			auto mouse = SDL_GetMouseState(&x, &y);
			for (auto& unitStack : sim->data[tile::UNITGROUP]) {
				std::stringstream txt;
				for (auto& unit : unitStack.second) {
					tile::Unit* ptr = static_cast<tile::Unit*>(unit);
					txt = std::stringstream{ "" };
					txt << "Unit " << std::hex << unit->UID << "\n";
					txt << "Status " << ptr->status << "\n";
					txt << "Position: " << ptr->position << "\n";
					if (ptr!=nullptr && ptr->destination.is_initialized())
						txt << "Destination: " << ptr->destination.get() << "\n";
					modern::Text&& unitTxt = modern::Text(
						context, txt.str(),
						expander.current.left() + expander.current.w*0.3, offset, font, SDL_Color{ 255, 255, 255, 255 });
					Zen::Areaf background = Zen::Areaf{ (double)unitTxt.x, (double)unitTxt.y, (double)unitTxt.w, (double)unitTxt.h, };
					if (background.contains(x, y))
					{
						SDL_SetRenderDrawColor(context->renderer, 0, 0, 0, 64);
						SDL_RenderFillRect(context->renderer, background.getArea());
						if ((mouse&SDL_BUTTON(1)) > 0)
							sim->selectedUnit = ptr;
					}
					//background.getArea()
					unitTxt.draw();
					offset += 20 + unitTxt.h;
				}
			}
			offset += 20;
			for (auto& unitStack : sim->data[tile::FIREGROUP]) {
				std::stringstream txt;
				for (auto& fire : unitStack.second) {
					tile::Fire* ptr = static_cast<tile::Fire*>(fire);
					if (ptr->isRoot == false)
						continue;
					txt = std::stringstream{ "" };
					txt << "Fire " << ptr->region << "-" << ptr->regionID << "\n";
					txt << "Position: " << ptr->pos;
					txt << "Incidents: " << ptr->incidents;
					modern::Text&& renderTxt = modern::Text(
						context, txt.str(),
						expander.current.left() + expander.current.w*0.3, offset, font, SDL_Color{ 255, 255, 255, 255 });
					Zen::Areaf background = Zen::Areaf{ (double)renderTxt.x, (double)renderTxt.y, (double)renderTxt.w, (double)renderTxt.h, };
					//background.getArea()
					renderTxt.draw();
					offset += 20 + renderTxt.h;
				}
			}
		}
		return 1;
	}
};

class UI::MenuUI : public UI::Interface{
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
	std::vector<std::string> texts = { { "Cancel" } };

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
	MenuUI(UI::art::context* ctx, UI::info* cfg, Facet_Sim* sim_ref=nullptr) : Interface{ sim_ref }, size{ cfg->get<int>("iconSize") }
	{
		init();
		context = new Context(ctx->first, ctx->second);

		auto a = cfg->get_child("Background.Area");
		int x, y, w, h;
		x = 10;//a.get<int>("x");
		y = 10;//a.get<int>("y");
		w = a.get<int>("w");
		h = a.get<int>("h");

		int offset = 0;


		auto abilities = cfg->get_child_optional("Abilities");
		if (abilities.is_initialized())
		{
			for (auto i = abilities.get().begin(); i != abilities.get().end(); i++) {
				texts.push_back(i->first);
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

	virtual bool update(const UI::info* info = NULL) {
		int i = 0;
		if (!alive)
			return false;

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

		if (sim->selectedUnit == nullptr) {
			alive = false;
			return false;
		}

		bool consumed = false;
		int index = 0;
		for (auto icon : icons)
		{
			icon.second->update(x, y);
			if (icon.second->hovering && left==2){
				alive = false;
				if (sim->selectedUnit == nullptr)
					return false;
				consumed = true;
				std::string newStatus = texts.at(index);
				sim->selectedUnit->status = newStatus;
			}
			++index;
		}

		expansion += (1 - expansion)*0.1;
		return false;
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

UI::Interface* UI::makeDetail(UI::info& cfg, UI::art::context& ctx, Facet_Sim* sim_ref)
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
		return new MenuUI(&ctx, &cfg, sim_ref);
	}
	else if (type == "List")
	{
		return new ListUI(&ctx, &cfg, sim_ref);
	}
	else {
		return new BasicImplementation(ctx, &cfg);
	}
}



UI::UI(UI::art::context& ctx, UI::info& cfg, Facet_Sim* sim_ref) : detail{ makeDetail(cfg, ctx, sim_ref) }
{
	type = cfg.get<std::string>("Type");
	detail->renderBackground();
}
UI::~UI(){ type = "empty"; }
void UI::draw()
{
	detail->draw();
}
bool UI::update(const UI::info* newData)
{
	return detail->update(newData);
}
bool UI::isAlive() {
	return detail->alive;
}
void UI::isAlive(bool setting) {
	detail->alive = setting;
}