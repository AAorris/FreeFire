#pragma once
#include "FACET_GLOBALS.h"
#include "Facet.h"
#include "Tool_Asset.h"
#include "scalar.h"
#include "camera_data.h"
//#include "Facet_Sim.h"
#include "SDLButton.h"
#include "Tool_Data.h"
#include <iosfwd>
#include <unordered_set>
#include <SDL2\SDL_ttf.h>

//Now SDL objects stored in things like unique_ptr can be automatically cleaned up.
namespace std {
	template<> struct default_delete<SDL_Renderer> {
		void operator()(SDL_Renderer* r){ SDL_DestroyRenderer(r); }
	};
	template<> struct default_delete<SDL_Window> {
		void operator()(SDL_Window* w){ SDL_DestroyWindow(w); }
	};
	template<> struct default_delete<SDL_Texture> {
		void operator()(SDL_Texture* t) { SDL_DestroyTexture(t); }
	};
	template<typename T>
	vector<T> range(int n, int begin = 0) {
		vector<T> result;
		result.reserve(n - begin);
		for (int i = begin; i < n; i++)
			result.push_back(T(i));
		return result;
	}
	template<typename T>
	class storage {
	public:
		using key = unsigned long;
		using value = T;
		using type = vector<value>;
		key gen() {
			content.reserve(content.size() + 1);
			return content.size();
		}
		vector<key> genv(int n) {
			content.reserve(content.size() + n);
			return range<key>(n, content.size());
		}
		value& get(const key& k) {
			return content.at(k);
		}
	private:
		vector<value> content;
	};
	namespace sdl {
		using renderer = unique_ptr<SDL_Renderer>;
		using window   = unique_ptr<SDL_Window>;
		using texture  = unique_ptr<SDL_Texture>;
		using font	   = unique_ptr<TTF_Font>;
	}
}

namespace ff {
	namespace gfx {
		class Context {
			using storage = std::storage<std::sdl::texture>;
			/*Texture storage. Stores unique_ptrs to loaded textures.*/
			storage _textures;
			std::sdl::window _window;
			std::sdl::renderer _renderer;
		public:
			Context();
			Context(SDL_Window* window, SDL_Renderer* renderer);
			/*Load a texture from path. Put in texture storage. Return key to texture*/
			storage::key loadTexture(const std::string& path);
			/*Store a loaded texture*/
			storage::key loadTexture(SDL_Texture* raw_ptr);
			/*Gets a texture based on its key (generated by loadTexture)*/
			storage::value getTexture(const storage& key) const;
			SDL_Window* getWindow();
			SDL_Renderer* getRenderer();
		};
	}
}

inline ff::gfx::Context::Context() :
_window{ SDL_CreateWindow("Window", 0, 0, 1024, 768, 0) },
_renderer{ SDL_CreateRenderer(_window.get(),-1,SDL_RENDERER_TARGETTEXTURE|SDL_RENDERER_ACCELERATED) } {

}

inline ff::gfx::Context::Context(SDL_Window* window, SDL_Renderer* renderer) :
_window{ window },
_renderer{ renderer } {

}

class Facet_Gfx :
	public Facet
{
private:
	struct Impl;
	std::unique_ptr<Impl> p;


	using master_type = facet::master_type;
	using group_type = facet::master_group_type;
	using ptree = facet::ptree;
	using template_key = facet::template_key;
	using template_type = facet::template_type;
public:

	Facet_Gfx(scalar size, bool relativeToScreen = false);
	virtual ~Facet_Gfx();
	Facet_Gfx(const Facet_Gfx& c) = delete;


	const _asset* getAsset(const tile::id_type& key);

	void connect(_cfg& session);
	void draw(const char id, const scalar& pos);
	void draw(master_type& data);
	void draw(SDLButton* b);
	void drawOverview(master_type& data);
	scalar getCell(const scalar& mouse);
	void highlightCell(const scalar& pos, SDL_Color& col = SDL_Color{ 255, 0, 0, 255 });
	void fillCell(const scalar& pos, SDL_Color& col);
	void drawTerrain(facet::master_group_type* terrain, facet::master_group_type* objects);
	//void connect(Tool_Data* to);
	//void draw(Tool_Data* data);
	std::pair<SDL_Window*, SDL_Renderer*> context();
	void present();
	void clear();
	void resize(int x, int y);
	void loadAsset(const std::string& path, int id);
	double getZoom();

	void zoomCamera(const double& dz);
	void moveCamera(const scalar& dp);
	cameraTool_Data getCamera();

	//const _asset& getAsset(const std::string& asset) const;
	//const _asset& getAsset(const int& asset) const;
};

typedef Facet_Gfx _gfx;
