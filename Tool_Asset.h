#pragma once
#include "Tool.h"
class Tool_Asset :
	public Tool
{
private:
	class Impl;
	std::unique_ptr<Impl> p;
public:

	Tool_Asset(const std::string& s, void* ren);
	virtual ~Tool_Asset();
	Tool_Asset(const Tool_Asset& c) = delete;
	void draw(int x, int y);
}; 