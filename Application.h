#pragma once
#include "Tool_UIElement.h"
#include "Facet_UI.h"

class Application
{
public:

	Facet_UI uis;
	long nextUpdateTicks;

	Application();
	virtual ~Application();
	Application(const Application& c) = delete;

	void run();
};

