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
	int w;
	int h;
	
	Impl(std::string p_path, SDL_Renderer* p_ren)
	{
		path = p_path;
		renderer = p_ren;
		auto ptr = sdlWrap(IMG_Load(p_path.c_str()));
		if (ptr.get() == NULL){
			std::stringstream error;
			error << "Error, couldn't find this asset : " << path;
			SDL_ShowSimpleMessageBox(0, "Asset Missing", error.str().c_str(), NULL);
		}
		texture = SDL_CreateTextureFromSurface(p_ren, ptr.get());
		SDL_QueryTexture(texture, NULL, NULL, &w, &h);
	}
	~Impl(){ if(texture!=NULL) SDL_DestroyTexture(texture); }

	void draw(int x, int y) const
	{
		if (renderer != NULL) {
			draw(x, y, -1, -1);
		}
		else {
			throw std::runtime_error("Asset not given renderer");
		}
	}

	void draw(int _x=-1, int _y=-1, int _w=-1, int _h=-1, bool centered = false) const
	{
		SDL_Rect r = {
			(_x == -1) ? x : _x,
			(_y == -1) ? y : _y,
			(_w == -1) ? w : _w,
			(_h == -1) ? h : _h
		};

		if (centered) {
			r.x -= r.w / 2;
			r.y -= r.h / 2;
		}

		if (SDL_RenderCopy(renderer, texture, NULL, &r)==0) return;
	}

	void draw(SDL_Rect* rect) const
	{
		if (SDL_RenderCopy(renderer, texture, NULL, rect) == 0) return;
	}

}; 

CTOR(const std::string& path, void* ren = NULL) :
	p{ new IMPL{ path, static_cast<SDL_Renderer*>(ren) } }
{
}

DTOR()
{
}

int CLASS::getSize() const
{
	return (p->w + p->h) / 2;
}

void CLASS::draw(int x=-1, int y=-1, int w=-1, int h=-1, bool centered = false) const
{
	p->draw(x, y, w, h, centered);
}
void CLASS::draw(SDL_Rect* rect) const
{
	p->draw(rect);
}

bool CLASS::operator==(const CLASS& c) const
{
	return p->path == c.getPath();
}

CLASS::CLASS(CLASS&& other) : p{ std::move(other.p) }
{
}

SDL_Texture* CLASS::getTexture() const {
	return p->texture;
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