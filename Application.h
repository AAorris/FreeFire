#pragma once
#include "Tool_UIElement.h"
class Application
{
public:

	std::vector<UI*> activeUIs;

	Application();
	virtual ~Application();
	Application(const Application& c) = delete;

	void run();
};

