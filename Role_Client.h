#pragma once
#include "Role.h"
#include "Facet_Cfg.h"
#include "Facet_Gfx.h"
#include "Facet_Net.h"

class Role_Client :
	public Role
{
private:
	class Impl
	{
		
	};
public:
	Role_Client();
	Role_Client(const Role_Client& c) = delete; 
	virtual ~Role_Client();

	void init();
	void start();
	void finish();
};

