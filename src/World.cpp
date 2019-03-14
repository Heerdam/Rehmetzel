#include "World.hpp"

#include <math.h>

using namespace Heerbann;

WorldOut * WorldBuilder::build(const WorldBuilderDefinition& _def) {
	WorldOut* out = new WorldOut();

	const float PI = (float)std::atan(1.0) * 4;
	const float toRad = 2 * PI / 360.f;

	int cells = 50;
	float diam = 100;

	int vertex = cells * cells;

	float* data = new float[3 * vertex];

	float xw = cosf(toRad * 30.f) * diam;
	float yw = 1.5f * diam;

	
	for (int y = 0; y < cells; ++y) {
		for (int x = 0; x < cells; ++x) {
		
			int index = 3 * (y*cells + x);

			data[index] = x * 2 * xw - ((y % 2 == 0) ? xw : 0);
			data[index + 1] = y * yw;
			data[index + 2] = (y % 2 == 0) ? 3.f : 4.f;
			
		}
	}

	out->bgs = new float*[1];
	out->bgs[0] = data;
	out->vertexcount = vertex;

	return out;
}
