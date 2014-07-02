#pragma once
#include "Facet.h"
class Facet_Net :
	public Facet
{
public:
	Facet_Net();
	virtual ~Facet_Net();
	Facet_Net(const Facet_Net& c) = delete;
};

