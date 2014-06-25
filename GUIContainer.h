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

template <typename TC>
class GUIContainer
{
private:
	int w;
	int h;
public:
	int x;
	int y;
	std::vector<TC> children;
	int spacing;
	CONTAINERTYPE layout;

	GUIContainer()
	{
		w = 0;
		h = 0;
		spacing = 0;
		children = std::vector<TC>();
		layout = CONTAINER_VERTICAL;
	}
	void add(const TC& child)
	{
		children.push_back(child);
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