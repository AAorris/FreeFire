#pragma once
#include "Role.h"
class Role_Server :
	public Role
{
public:
	Role_Server();
	virtual ~Role_Server();
	Role_Server(const Role_Server& c) = delete;
};

