#include "stdafx.h"
#include "Tool_Asset.h"
#include <SDL2\SDL_render.h>
#include <SDL2\SDL_image.h>

#define CLASS Tool_Asset
#include "PIMPL.h"

//#define DEF Tool_Asset //these get UNDEF'ed
//#define IMPL Tool_Asset::Impl //DONT FORGET THE UNDEF :-)
std::vector<std::string> Tool_Asset::lookup = std::vector<std::string>();
/*
std::function<SDL_Surface* delSurf(SDL_Surface* s)
{
	SDL_FreeSurface(s);
}*/

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
	SDL_Texture* texture = NULL;
	SDL_Renderer* renderer = NULL;
	std::string path;

	int x;
	int y;

	Impl(std::string p_path, SDL_Renderer* p_ren)
	{
		lookup.push_back(p_path);
		path = p_path;
		renderer = p_ren;
		auto ptr = sdlWrap(IMG_Load(p_path.c_str()));
		texture = SDL_CreateTextureFromSurface(p_ren, ptr.get());
	}
	~Impl(){ if(texture!=NULL) SDL_DestroyTexture(texture); }

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

CTOR(const std::string& path, void* ren) :
	p{ new IMPL{ path, (SDL_Renderer*)ren } },
	id{ lookup.size() }
{

}

DTOR()
{
}

int CLASS::useLookup(const std::string& s)
{
	auto it = std::find(begin(lookup), end(lookup), s);
	auto dist = std::distance(begin(lookup), it);
	return (dist==lookup.size())?-1:dist;
}

void CLASS::draw(int x, int y)
{
	p->draw(x, y);
}

bool CLASS::operator==(const CLASS& c) const
{
	return c.id == id;
}

CLASS::CLASS(CLASS&& other) : p{ std::move(other.p) }
{
	id = std::move(other.id);
	path = std::move(other.path);
}

/*
void delTex(SDL_Texture* t) {
	SDL_DestroyTexture(t);
	delete t;
}
*/


//#undef DEF
//#undef IMPL