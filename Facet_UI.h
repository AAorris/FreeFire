#pragma once
#include "Facet.h"
#include "Tool_UIElement.h"
#include "Tool_Configurable.h"
class Facet_UI :
	public Facet
{
public:
	using cfg = Tool_Configurable;
	std::vector<UI*> elements;
	Facet_UI();
	virtual ~Facet_UI();
	Facet_UI(const Facet_UI& c) = delete;
	bool update(UI::info* info, int ms);
	void connect(const cfg& session);
};

