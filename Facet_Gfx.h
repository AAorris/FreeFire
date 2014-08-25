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
	void drawTerrain(facet::master_group_type* terrain);
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


namespace SDL {
	template<> struct default_delete<SDL_Renderer> {
		void operator()(SDL_Renderer* s){ SDL_DestroyRenderer(s); }
	};
	template<> struct default_delete<SDL_Window> {
		void operator()(SDL_Window* s){ SDL_DestroyWindow(s); }
	};
}