#pragma once
#include "scalar.h"

struct cameraTool_Data {
	scalar _origin, _off;
	scalar _size;
	double _z, _res;
	cameraTool_Data(scalar origin, scalar offset, scalar size, double zoom, double res);
	scalar cell(scalar pos) const;
	std::pair<scalar, scalar> getRange(int paddingRings = 0) const;
};