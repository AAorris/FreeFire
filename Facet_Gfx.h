#pragma once
#include "Facet.h"
#include "Tool_Asset.h"
#include "Facet_Sim.h"
#include "scalar.h"
#include "camera_data.h"
#include <iosfwd>
#include <unordered_set>



class Facet_Gfx :
	public Facet
{
private:
	struct Impl;
	std::unique_ptr<Impl> p;
public:

	Facet_Gfx();
	virtual ~Facet_Gfx();
	Facet_Gfx(const Facet_Gfx& c) = delete;

	template <typename TKey, typename TReturn>
	const TReturn& getAsset(const TKey& key)
	{
		return p->find(key);
	}

	void draw(const int& key, int x=0, int y=0);
	void draw(const std::string& key, int x = 0, int y = 0);
	void draw(const AA::Pos& pos, char& id);
	void present();
	void clear();
	void loadAsset(const std::string& path, int id);

	void zoomCamera(const double& dz);
	void moveCamera(const scalar& dp);
	camera_data getCamera();

	//const _asset& getAsset(const std::string& asset) const;
	//const _asset& getAsset(const int& asset) const;
};

typedef Facet_Gfx _gfx;


namespace SDL {
	template<> struct default_delete<SDL_Renderer> {
		void operator()(SDL_Renderer* s){ SDL_DestroyRenderer(s); }
	};
	template<> struct default_delete<SDL_Window> {
		void operator()(SDL_Window* s){ SDL_DestroyWindow(s); }
	};
}