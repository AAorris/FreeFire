#include "stdafx.h"
#include "Tool_Asset.h"
#include <SDL2\SDL_render.h>
#include <SDL2\SDL_image.h>

#define CLASS Tool_Asset
#include "PIMPL.h"


class IMPL
{
private:
	//Don't let C++ throw around copies all willy nilly
	//or else the destructor will corrupt the pointer
	Impl(const Impl& asset) = delete;
	Impl() = delete;
protected:
public:
	SDL_Texture* texture = NULL;
	SDL_Renderer* renderer = NULL;
	std::string path;

	int x;
	int y;

	Impl(std::string p_path, SDL_Renderer* p_ren)
	{
		path = p_path;
		renderer = p_ren;
		auto ptr = sdlWrap(IMG_Load(p_path.c_str()));
		texture = SDL_CreateTextureFromSurface(p_ren, ptr.get());
	}
	~Impl(){ if(texture!=NULL) SDL_DestroyTexture(texture); }

	void draw(int x, int y) const
	{
		draw(x, y, -1, -1);
	}

	void draw(int x, int y, int w=-1, int h=-1) const
	{
		SDL_Rect rdst = { x, y, w, h };
		if (SDL_min(w,h)<1)
			SDL_QueryTexture(texture, NULL, NULL, &rdst.w, &rdst.h);
		if (SDL_RenderCopy(renderer, texture, NULL, &rdst)==0) return;
		else {
			enum UNINITIALIZED_ASSET{ ERROR };
			throw ERROR;
		}
	}

}; 

CTOR(const std::string& path, void* ren, int pid) :
	p{ new IMPL{ path, (SDL_Renderer*)ren } },
	id{pid}
{
}

DTOR()
{
}

void CLASS::draw(int x, int y, int w, int h) const
{
	p->draw(x, y, w, h);
}

bool CLASS::operator==(const CLASS& c) const
{
	return c.id == id;
}

CLASS::CLASS(CLASS&& other) : p{ std::move(other.p) }
{
	id = other.id;
}

std::string CLASS::getPath() const
{
	return p->path;
}

/*
void delTex(SDL_Texture* t) {
	SDL_DestroyTexture(t);
	delete t;
}
*/


//#undef DEF
//#undef IMPL