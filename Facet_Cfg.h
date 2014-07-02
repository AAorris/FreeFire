#pragma once
#include "Facet.h"
class Facet_Cfg :
	public Facet
{
public:
	Facet_Cfg();
	virtual ~Facet_Cfg();
	Facet_Cfg(const Facet_Cfg& c) = delete;
};

