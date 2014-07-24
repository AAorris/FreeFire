#pragma once
#include "Facet.h"
#include "Tool_Asset.h"
#include "scalar.h"
#include "camera_data.h"
#include "Tool_Data.h"
#include <iosfwd>
#include <unordered_set>



class Facet_Gfx :
	public Facet
{
private:
	struct Impl;
	std::unique_ptr<Impl> p;

	using group_type = std::unordered_map<scalar, tile::Data*>;
	//attempting to order lexicographically by id, and then position.
	//Imagine updating all trees, all houses, all units, all fires.
	//This should help with branch prediction when update checks id (maybe?)
	using master_type =
		std::unordered_map<
		tile::group_type,
		group_type
		>;
public:

	Facet_Gfx();
	virtual ~Facet_Gfx();
	Facet_Gfx(const Facet_Gfx& c) = delete;

	template <typename TKey, typename TReturn>
	const TReturn& getAsset(const TKey& key)
	{
		return p->find(key);
	}

	void connect(_cfg& session);
	void draw(const char& id, const scalar& pos);
	void draw(master_type& data);
	//void connect(Tool_Data* to);
	//void draw(Tool_Data* data);
	void present();
	void clear();
	void resize(int x, int y);
	void loadAsset(const std::string& path, int id);

	void zoomCamera(const double& dz);
	void moveCamera(const scalar& dp);
	cameraTool_Data getCamera();

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