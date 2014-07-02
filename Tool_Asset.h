#pragma once
#include "Tool.h"
class Tool_Asset :
	public Tool
{
public:
	Tool_Asset();
	virtual ~Tool_Asset();
	Tool_Asset(const Tool_Asset& c) = delete;
};

