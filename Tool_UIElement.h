#pragma once
#include "Tool.h"
class Tool_UIElement :
	public Tool
{
public:
	Tool_UIElement();
	virtual ~Tool_UIElement();
	Tool_UIElement(const Tool_UIElement& c) = delete;
};

