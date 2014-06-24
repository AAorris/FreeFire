#pragma once
#include <iostream>
#include <vector>
enum CONTAINERTYPE
{

	VERTICAL,
	HORIZONTAL,
	GRID,
};
template <typename T>
class GUIContainer
{
	int pos;
	T _head;
	std::vector<T> children;
	int w;
	int h;
	CONTAINERTYPE layout;
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

};