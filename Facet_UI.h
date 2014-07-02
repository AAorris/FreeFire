#pragma once
#include "Facet.h"
class Facet_UI :
	public Facet
{
public:
	Facet_UI();
	virtual ~Facet_UI();
	Facet_UI(const Facet_UI& c) = delete;
};

