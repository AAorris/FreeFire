#pragma once
#include "sprite.h"
#include <SDL2\SDL_events.h>
#include <SDL2\SDL_pixels.h>

class SDLButton
{
public:
	
	SDLButton(void);
	SDLButton(int x, int y, int w, int h, std::string label);
	~SDLButton(void);
	void init();
	void init(std::string label, std::string spritesheet, int x, int y, int w, int h);
	void destroy();

	std::string		sheetName;
	std::string		textLabel;
	SDL_Texture*	t_label;
	SDL_Rect		r_label;
	int				x;
	int				y;
	int				w;
	int				h;
	bool			hovering;
	bool			clicking;

	SDL_Color idleCol;
	SDL_Color hoverCol;
	SDL_Color clickCol;

	void update();
};

