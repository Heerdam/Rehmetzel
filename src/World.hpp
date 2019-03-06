#ifndef WORLD_H
#define WORLD_H

#include <Box2D/Box2D.h>

#include "MainStruct.hpp"

namespace Heerbann {

	using namespace Heerbann;

	class MainStruct;

	struct Entity {

	};

	class World {

		b2World* bworld = new b2World(b2Vec2(0, -9.8f), true);

		void update(float _deltaTime){  
  			bworld->Step( 1.f/60.f, 8, 3);
		}


	};

}

#endif