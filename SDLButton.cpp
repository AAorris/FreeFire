#include "stdafx.h"
#include "SDLButton.h"


SDLButton::SDLButton(void)
{
	init();
}

SDLButton::SDLButton(int x, int y, int w, int h, std::string label)
{
	init(label, "", x, y, w, h);
}

SDLButton::~SDLButton(void)
{
}

void SDLButton::init()
{
	sheetName = "";
	textLabel = "";
	w = 0;
	h = 0;
	hovering = false;
	clicking = false;
	x = 0;
	y = 0;
	idleCol = { 128, 128, 128, 40 };
	hoverCol = { 32, 32, 32, 40 };
	clickCol = { 0, 0, 0, 64 };
	t_label = NULL;
}

void SDLButton::init(std::string label, std::string spritesheet, int _x, int _y, int _w, int _h)
{
	init();
	textLabel = label;
	sheetName = spritesheet;
	x = _x;
	y = _y;
	w = _w;
	h = _h;
	t_label = NULL;
}

void SDLButton::destroy()
{
	SDL_DestroyTexture(t_label);
}

void SDLButton::update()
{
	int mx = 0;
	int my = 0;
	Uint32 state = SDL_GetMouseState(&mx, &my);

	//check for mouse hover
	hovering = false;
	if (mx > x && mx < x + w)
	{
		if (my > y && my < y + h)
		{
			hovering = true;
		}
	}

	//check for mouse click(only if hovering)
	if (hovering && ((state&SDL_BUTTON(SDL_BUTTON_LEFT)) > 0))
		clicking = true;
	else
		clicking = false;
}