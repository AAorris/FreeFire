#pragma once

#include "AAUI.h"

class LinearBar : public AAUI {
public:
	float val;
	float max;
	float min;
	float progress;
	void init() {
		val = 0;
		min = 0;
		max = 1;
	}
	void setValue(float f) {
		val = f;
	}
	void update() {
		progress = (val - min) / (max - min);
	}
	void draw() {
	}
};