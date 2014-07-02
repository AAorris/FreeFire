#pragma once
#include "Tool.h"
class Tool_Messenger :
	public Tool
{
public:
	Tool_Messenger();
	virtual ~Tool_Messenger();
	Tool_Messenger(const Tool_Messenger& c) = delete;
};

