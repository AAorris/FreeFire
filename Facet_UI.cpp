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


void Facet_UI::update(UI::info* info, int ms)
{
	for (auto& ui : elements)
	{
		ui->update(info);
	}
}

void Facet_UI::connect(const cfg& session)
{

}