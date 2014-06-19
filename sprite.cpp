#include "sprite.h"

animation::animation(std::vector<int> p_frames, int p_framerate)
{
	frames=p_frames;
	framerate=p_framerate;
}

animation::animation()
{
	frames = std::vector<int>();
	framerate = 12;
}

sprite::sprite(
		std::string p_sheetPath,
		int p_cellw, int p_cellh
		)
{
	cw=p_cellw;
	ch=p_cellh;
	iw=0;
	frame=0;
	ih=0;
	sheet = p_sheetPath;
	subframe=0;
	time=0;
}

sprite::~sprite() {}
void sprite::play() {}
void sprite::play(std::string key)
{
	cur = &animations[key];
	subframe=0;
	frame = cur->frames[0];
}
void sprite::update(int step) {
	time+=step;
	int frameMs = 1000/cur->framerate;
	while( time > frameMs )
	{
		subframe++;
		time-=frameMs;
	}
	subframe %= cur->frames.size();
}
void sprite::addAnimation(std::string name, std::vector<int> p_frames, int p_framerate) {
	animation anim = animation(p_frames,p_framerate);
	//animations.insert(std::make_pair<std::string,animation>(name,anim));
	animations.insert(std::make_pair(name,anim));
}