#pragma once
#include "Tool.h"
#include <SDL2\SDL_render.h>
#include <boost\property_tree\ptree_fwd.hpp>
#include "Facet_Sim.h"


/*The UI class extends an interface class. Inside the cpp file are different ui implementations.
The constructor chooses which implementation to use based on the config.*/
class UI : protected Tool
{
public:
	//---------- 
	using info = boost::property_tree::ptree;
	struct art {
		using artist = SDL_Renderer*;
		using frame = SDL_Window*;
		using context = std::pair<frame, artist>;
		using canvas = SDL_Texture*;
		using area = SDL_Rect;
	};
	/*Contains a canvas(texture), and a context(Window,Renderer)*/
	struct Image {
		art::canvas canvas = NULL;
		art::context context;
		art::area area;
		SDL_Point center();
		Image(art::context p_context, int const& w, int const& h);
		Image(art::context p_context, const info& area);
		Image();
		~Image() = default;
	};
	//----------
	class Interface;
private:
	class BasicImplementation;
	class CompassUI;
	class MenuUI;
	class ListUI;
	std::unique_ptr<Interface> detail;
	Interface* makeDetail(UI::info& cfg, UI::art::context& ctx, Facet_Sim* sim_ref = nullptr);
public:
	/*Describes that the UI is running and should stay in the menu*/
	void isAlive(bool setting);
	bool isAlive();
	std::string type;

	UI(art::context& ctx, info& cfg, Facet_Sim* sim_ref = nullptr);
	/*This class deals with unique ptrs, so it should stay unique. Transfer with move...*/
	void operator=(const UI& other) = delete;
	~UI();
	//bool isPoppedOut();
	//void setIsPoppedOut(bool popped);

	void draw();
	void update(const info* data);
	UI::Image getTexture();
};