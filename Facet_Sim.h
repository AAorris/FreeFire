#pragma once
#include "Facet.h"
class Facet_Sim :
	public Facet
{
private:
	class Impl;
	std::unique_ptr<Impl> p;
	Facet_Sim(const Facet_Sim& c) = delete;
	void operator=(const Facet_Sim& f) = delete;
public:
	Facet_Sim();
	virtual ~Facet_Sim();

	void update();
	/**
	Should return a simulation layer for drawing. The return
	value should be an id on the graphics module's reference
	table.
	*/
	std::map<int, int> getLayer(const std::string& layer);

	//save the simulation state for a specific time to a file
	void loadState(const std::string& path);
	void saveState(const std::string& path);
	
	//each session sim should have a tree built up of
	// layer(id) > template(id) > instance(pos)
	void addItem(const int& layerID, const int& templateID, const int& pos);

	//get a serialized set of items (impl dependent, relevant chunks or single items?)
	//in the relevant area of radius+pos (square or circle?)
	//and only does so if the hash is either -1, or equal to the hash of the same area
	int hashArea(const int& pos, const int& radius);
	std::string getItems(const int& pos, const int& radius=-1, const int& hash=-1);

	//void loadSession(const std::string& path);
	//void loadTemplate(const std::string& path);
	//void loadInstance(const std::string& path);
	
};

typedef Facet_Sim t_sim;