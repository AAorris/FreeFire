#include "stdafx.h"
#include "Tool_Asset.h"
#include <SDL2\SDL_render.h>
#include <SDL2\SDL_image.h>

#define DEF Tool_Asset //these get UNDEF'ed
#define IMPL DEF::Impl //DONT FORGET THE UNDEF :-)

void delTex(SDL_Texture* t) {
	SDL_DestroyTexture(t);
	delete t;
}

struct IMPL::asset
{
	SDL_Texture* texture;
	SDL_Renderer* renderer;
	std::string path;

	asset(std::string p_path, SDL_Renderer* p_ren)
	{
		path = p_path;
		renderer = p_ren;
		SDL_Surface* temp = IMG_Load(path.c_str());
		texture = SDL_CreateTextureFromSurface(renderer,temp);
		SDL_FreeSurface(temp);
	}

	//Don't let C++ throw around copies all willy nilly
	//or else the destructor will corrupt the pointer
	asset(const asset& asset) = delete;
	void operator=(const asset& asset) = delete;
	~asset(){SDL_DestroyTexture(texture);}
};

Tool_Asset::Tool_Asset(const std::string& path)
{

}


Tool_Asset::~Tool_Asset()
{
}


#undef DEF
#undef IMPL