#pragma once
#include "Tool.h"
#include <SDL2\SDL_render.h>
class Tool_UIElement :
	public Tool
{
private:
	class UI;
	std::unique_ptr<UI> impl;
public:
	explicit Tool_UIElement(SDL_Renderer* renderer);
	explicit Tool_UIElement(Tool_UIElement&& t);
};

using _ui = Tool_UIElement;