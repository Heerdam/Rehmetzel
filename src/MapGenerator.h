#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	class MapGenerator {

		const float TILESIZE = 512.f;
		
	public:

		struct MapDefinition {
			int width = 1000, height = 1000;
			float radius;
		};

		void create(const MapDefinition& _def) {

			float* vertex = new float[_def.height * _def.width * 2];
			char* index = new char[_def.height * _def.width]{ 0 }; //not index buffer.

			for (int y = 0; y < _def.height; ++y) {
				for (int x = 0; x < _def.width; ++x) {		
					int index = (x + y * _def.width) * 2;
					vertex[index] = TILESIZE * x;
					vertex[index + 1] = TILESIZE * y;
				}
			}




		};

		void bind() {

		};

		void unbind() {

		};

		void draw() {

		};

	};

}