#include "stdafx.h"
#include "Facet_UI.h"
#include <SDL2\SDL_thread.h>
#include <SDL2\SDL_mutex.h>

Facet_UI::Facet_UI()
{
}


Facet_UI::~Facet_UI()
{
}


bool Facet_UI::update(UI::info* info, int ms)
{
	bool consumed = false;
	if (info->empty() == false && info!=nullptr) {
		for (auto& ui : elements)
		{
			consumed |= ui->update(info);
		}
	}
	return consumed;
}

void Facet_UI::connect(const cfg& session)
{

}