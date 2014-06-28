#pragma once
#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <map>
#include <SDL2\SDL_rect.h>

enum CONTAINERTYPE
{
	CONTAINER_VERTICAL,
	CONTAINER_HORIZONTAL,
	CONTAINER_GRID,
};

template <typename T>
class GUIContainer
{
private:
	int w;
	int h;
public:
	int x;
	int y;
	int selectedIndex;
	bool selectingIndex;
	std::vector<T> children;
	std::vector<SDL_Rect> cachedRects;
	std::vector<SDLButton> buttons;
	int spacing;
	CONTAINERTYPE layout;

	GUIContainer()
	{
		w = 0;
		h = 0;
		selectedIndex = -1;
		selectingIndex = false;
		spacing = 0;
		children = std::vector<T>();
		layout = CONTAINER_VERTICAL;
	}
	void add(const T& child)
	{
		children.push_back(child);
	}
	//draw(T,rect)
	void draw( void* func)
	{
		for (int i = 0; i < children.size(); i++)
		{
			func(children[i], cachedRects[i]);
		}
	}
	SDL_Rect background_rect()
	{
		SDL_Rect r = { x, y, w, h };
		return r;
	}
	std::vector<SDL_Rect> icon_rects()
	{
		//for now forcing horizontal layout
		std::vector<SDL_Rect> rects;
		//int avail_w = w - (spacing * 2);
		int avail_h = h - (spacing * 2);
		//spacing on each side, and in between each child
		int avail_w = (w - (spacing * 2) - spacing*children.size() - 1);
		int ideal_w = avail_w / children.size();
		int size = (ideal_w < avail_h) ? ideal_w : avail_h;
		SDL_Rect base = { spacing, spacing, size, size };
		for (int i = 0; i < children.size(); i++)
		{
			rects.push_back(base);
			base.x += spacing;
			base.x += size;
		}
		return rects;
	}
	void setWidth(const int& width)
	{
		w = width;
		layout = CONTAINER_HORIZONTAL;
		cachedRects = icon_rects();
	}
	void setHeight(const int& height)
	{
		h = height;
		layout = CONTAINER_VERTICAL;
	}
	void setSize(const int& width, const int& height)
	{
		w = width;
		h = height;
		layout = CONTAINER_GRID;
	}
	void setSpacing(const int& p_spacing)
	{
		spacing = p_spacing;
	}
};

/*
template <typename T>
class GUIContainer
{
	int pos;
	T _head;
	std::vector<T> children;
	int w;
	int h;
	//CONTAINERTYPE layout;
	GUIContainer()
	{
		pos = 0;
		head = NULL;
		children = std::vector < T > ();
	}
	const T& cur()
	{
		return children[pos];
	}
	const T& head()
	{
		return _head;
	}
	const T& next()
	{
		pos++;
		return cur();
	}
	const T& prev()
	{
		pos--;
		return cur();
	}

};*/