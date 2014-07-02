#pragma once
#include "Role.h"
class Role_TeamBoard :
	public Role
{
public:
	Role_TeamBoard();
	virtual ~Role_TeamBoard();
	Role_TeamBoard(const Role_TeamBoard& c) = delete;
};

