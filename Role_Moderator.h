#pragma once
#include "Role.h"
class Role_Moderator :
	public Role
{
public:
	Role_Moderator();
	virtual ~Role_Moderator();
	Role_Moderator(const Role_Moderator& c) = delete;
};

