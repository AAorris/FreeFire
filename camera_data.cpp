#include "stdafx.h"
#include "camera_data.h"

cameraTool_Data::cameraTool_Data(scalar origin, scalar offset, scalar size, double zoom, double res)
{
	_size = size;
	_origin = origin;
	_off = offset;
	_z = zoom;
	_res = res;
}
scalar cameraTool_Data::cell(scalar pos) const
{
	scalar result = pos / _res;
	result.x = result.x;
	result.y = result.y;
	return result;
}
std::pair<scalar, scalar> cameraTool_Data::getRange(int paddingRings) const
{
	scalar pad = scalar(1.0, 1.0)*paddingRings;
	//int xmin = _off.x - _size.x / 2;
	//int xmax = _off.x + _size.x / 2;
	//int ymin = _off.y - _size.y / 2;
	auto base = scalar(0, 0);
	auto baseOffset = _size / 2;
	auto scaleFactor = _z;

	auto min = cell(base - baseOffset / scaleFactor) + scalar(_off.x,-_off.y) - pad;
	auto max = cell(base + baseOffset / scaleFactor) + scalar(_off.x, -_off.y) + pad;

	//auto min = cell((_off - _size - pad)/_z);
	//auto max = cell((_off + _size + pad)/_z);
	return std::make_pair(min, max);
}