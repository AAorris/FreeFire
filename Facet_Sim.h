#pragma once
#include "Facet.h"
class Facet_Sim :
	public Facet
{
public:
	Facet_Sim();
	virtual ~Facet_Sim();
	Facet_Sim(const Facet_Sim& c) = delete;
};

