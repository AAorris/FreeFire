#pragma once

#include <SDL2\SDL.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>

struct animation
{
	animation(std::vector<int> p_frames, int p_framerate=12);
	animation();
	std::vector<int> frames;
	int framerate;
};

class sprite
{
public:
	sprite(
		std::string p_sheetPath,
		int p_cellw, int p_cellh
		);
	~sprite();
	std::string sheet;
	//animations
	std::map<std::string,animation> animations;
	animation* cur;
	//cell
	int cw, ch;
	//image
	int iw, ih;
	int frame;
	int subframe;
	int time;

	void update(int step);
	void addAnimation(std::string name, std::vector<int> p_frames, int p_framerate);
	void play();
	void play(std::string animKey);
};