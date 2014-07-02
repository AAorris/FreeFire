#pragma once
class Tool
{
public:
	Tool();
	virtual ~Tool();
	Tool(const Tool& c) = delete;
};

