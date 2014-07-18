#include "stdafx.h"
#include "scalar.h"
#include "Tool_UIElement.h"
#include <SDL2\SDL.h>
#include <SDL2\SDL_ttf.h>

class _ui::UI {
public:
	TTF_Font* font;
	scalar size;
	SDL_Renderer* renderer;
	SDL_Texture* output;
	SDL_Color bg = SDL_Color{ 0, 0, 0, 0 };
	SDL_Color fg = SDL_Color{ 60, 60, 60, 255 };

	UI(SDL_Renderer* ren)
	{
		renderer = ren;
		font = TTF_OpenFont("font/Roboto-Medium.ttf", 12);
		output = nullptr;
		size = scalar(0, 0);
	}
	~UI()
	{
		TTF_CloseFont(font);
		if (output != nullptr)
			SDL_DestroyTexture(output);
	}

	void create(int w, int h, SDL_TextureAccess type = SDL_TEXTUREACCESS_TARGET)
	{
		output = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, type, w, h);
		size.x = w;
		size.y = h;
	}

	void draw(int x=0, int y=0)
	{
		SDL_Rect r = { x, y, size.x, size.y };
		SDL_RenderCopy(renderer, output, NULL, &r);
	}

	SDL_Texture* operator*() {
		return output;
	}
};

Tool_UIElement::Tool_UIElement(SDL_Renderer* renderer) : impl { new UI(renderer) }
{

}
