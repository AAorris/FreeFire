#pragma once

#include <iosfwd>

class Role
{
private:
	class Impl
	{

	};
public:
	Role();
	virtual ~Role();
	Role(const Role& c) = delete;
};

