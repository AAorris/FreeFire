#pragma once

#define BLACK {0,0,0,255}
#define WHITE {255,255,255,255}
#define ALPHA {0,0,0,0}

#include <SDL2\SDL.h>
#include <SDL2\SDL_ttf.h>
#include <SDL2\SDL_image.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include "sprite.h"
#include "SDLButton.h"

class GFX {
public:
	SDL_Window* win;
	SDL_Renderer* ren;
	TTF_Font* font;
	std::unordered_map<std::string, SDL_Texture*> textures;
	std::unordered_map<std::string, _TTF_Font*> fonts;

	GFX();
	~GFX();
	void init();
	void init(std::string name, int w, int h, bool borderless=false);
	void cleanup();

	int w, h;

	SDL_Texture* getText(std::string content, std::string fontKey);
	void fill(int r, int g, int b, int a);
	void fill(std::string assetName);
	void fill(SDL_Texture* texture);
	void drawImage(SDL_Texture* t, int x, int y);
	void loadSprite(sprite& s);
	void drawSprite(sprite& s, int x, int y);
	void write(std::string text, int x, int y);
	void loadAsset(std::string key, std::string path);
	void loadFont(std::string key, std::string path, int fontSize);
	void fontWrite(std::string font, std::string text, int x, int y);
	void drawButton(SDLButton& b);
	void show();
};