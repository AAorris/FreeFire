#pragma once
#include "Tool.h"
#include <SDL2\SDL_render.h>
#include <boost\property_tree\ptree_fwd.hpp>


/*This will be a standard UI Element. UI Elements have a background image*, are capable of being drawn in or individual windows, */
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
private:
	class Interface;
	class BasicImplementation;
	class CompassUI;
	class SelectionUI;
	std::unique_ptr<Interface> detail;
	Interface* makeDetail(UI::info& cfg, UI::art::context& ctx);

public:
	UI(art::context& ctx, info& cfg);
	/*This class deals with unique ptrs, so it should stay unique. Transfer with move...*/
	void operator=(const UI& other) = delete;
	~UI();
	//bool isPoppedOut();
	//void setIsPoppedOut(bool popped);

	UI::Image* background;

	void draw();
	void update(info* data);
	UI::Image getTexture();
};