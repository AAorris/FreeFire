#include "stdafx.h"
#include "Tool_Asset.h"
#include <SDL2\SDL_render.h>
#include <SDL2\SDL_image.h>

#define DEF Tool_Asset //these get UNDEF'ed
#define IMPL Tool_Asset::Impl //DONT FORGET THE UNDEF :-)


class IMPL
{
private:
	//Don't let C++ throw around copies all willy nilly
	//or else the destructor will corrupt the pointer
	Impl(const Impl& asset) = delete;
	void operator=(const Impl& asset) = delete;
	Impl() = delete;
protected:
public:
	SDL_Texture* texture;
	SDL_Renderer* renderer;
	std::string path;

	int x;
	int y;

	Impl(std::string p_path, SDL_Renderer* p_ren)
	{
		path = p_path;
		renderer = p_ren;
		SDL_Surface* temp = IMG_Load(path.c_str());
		texture = SDL_CreateTextureFromSurface(renderer, temp);
		SDL_FreeSurface(temp);
	}
	~Impl(){ SDL_DestroyTexture(texture); }

	void draw(int x, int y)
	{
		SDL_Rect rdst = { x, y, 0, 0 };
		if (texture != NULL && renderer != NULL)
		{
			SDL_QueryTexture(texture, NULL, NULL, &rdst.w, &rdst.h);
			SDL_RenderCopy(renderer, texture, NULL, &rdst);
		}
		else {
			enum UNINITIALIZED_ASSET{ ERROR };
			throw ERROR;
		}
	}

};

Tool_Asset::Tool_Asset(const std::string& path, void* ren) :
	p { new IMPL{ path, (SDL_Renderer*)ren } }
{

}

DEF::~DEF()
{
}

void DEF::draw(int x, int y)
{
	p->draw(x, y);
}

/*
void delTex(SDL_Texture* t) {
	SDL_DestroyTexture(t);
	delete t;
}
*/

#undef DEF
#undef IMPL