#pragma once
#include <set>
class Module
{
public:
	Module();
	~Module();

	virtual std::set<std::string> getNews() = 0;
	virtual void loadState(const std::string& s) = 0;
};

