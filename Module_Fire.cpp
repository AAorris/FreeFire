#include "stdafx.h"
#include "Module_Fire.h"
#include "Tool_Pos.h"
#include <fstream>
#include <algorithm>
//#include <iostream>
#include <set>


struct t_fire : AA::Pos
{
	int time = 0;
	t_fire(int const& x, int const& y) : AA::Pos(x, y)
	{}
	t_fire(const AA::Pos& p) : AA::Pos(p)
	{}
};

class Fire::Impl
{
	std::set<unique_ptr<t_fire>> fire;
	std::set<AA::Pos> _news;
	int burnTime = 1000 * 3;
public:
	Impl();
	~Impl();
	void setFire(const AA::Pos& where);
	void update(int step=1000);
	std::set<std::string> news();
};

Fire::Impl::Impl(){}
Fire::Impl::~Impl(){}

std::set<std::string> Fire::Impl::news()
{
	auto res = std::set<std::string>{};
	std::stringstream ss;
	for (auto i : _news)
	{
		ss << "F " << i.serialize() << "\n";
		res.insert(ss.str());
		ss.str("");
	}
	_news.clear();
	return res;
}

void Fire::Impl::setFire(const AA::Pos& where)
{
	for (const std::unique_ptr<t_fire>& i : fire)
	{
		if (i->xy() == where.xy())
			return;
	}
	fire.insert(wrap( new t_fire(where) ));
	_news.insert(where);
}

void Fire::Impl::update(int step)
{
	for (auto& i : fire)
	{
		if (i->time!=-1)
			i->time+=step;
		if (i->time > burnTime)
		{
			i->time = -1;
			setFire(t_fire(i->x() + 1, i->y() + 1));
			setFire(t_fire(i->x() + 0, i->y() + 1));
			setFire(t_fire(i->x() + -1, i->y() + 1));
			setFire(t_fire(i->x() + 1, i->y() + 0));
			setFire(t_fire(i->x() + -1, i->y() + 0));
			setFire(t_fire(i->x() + 1, i->y() + -1));
			setFire(t_fire(i->x() + 0, i->y() + -1));
			setFire(t_fire(i->x() + -1, i->y() + -1));
		}
	}
}

Module_Fire::Module_Fire() : p{ new Fire::Impl()}
{

}

Module_Fire::Module_Fire(const Module_Fire& f) : p{ new Fire::Impl() }
{

}


Module_Fire::~Module_Fire()
{
}

void Fire::startFire(const AA::Pos& where)
{
	p->setFire(where);
}

void Fire::loadState(const std::string& fp)
{
	std::ifstream f {fp};
	short x, y;
	std::string chunk;

	if (f >> x)
	{
		f >> y;
		AA::Pos cpos{ x*CHUNK::width, y*CHUNK::height };
		f >> chunk;
		for (int i = 0; i < chunk.size(); i++)
		{
			char c = chunk[i];
			AA::Pos tpos{ cpos.x()+i%CHUNK::width, cpos.y()/CHUNK::width };
			if (c == 'F')
				startFire(tpos);
		}
	}
}

void Fire::update(const int& step)
{
	p->update(step);
}

std::set<std::string> Fire::getNews()
{
	auto news = p->news();
	if (news.size() > 0)
		int yay = 1;
	return news;
}