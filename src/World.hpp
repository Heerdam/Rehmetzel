#ifndef WORLD_H
#define WORLD_H

#include <Box2D/Box2D.h>

#include "MainStruct.hpp"

namespace Heerbann {

	using namespace Heerbann;

	enum EntityType {
		red_deer, tree
	};

	class World {

		b2World* bworld = new b2World(b2Vec2(0, -9.8f));

		void update(float _deltaTime){  
  			bworld->Step(_deltaTime, 8, 3);
		}

		struct WorldObject {
			long id = MainStruct::getId();
			b2Body* body;
		};

		long create(EntityType _type) {
			WorldObject *ob = new WorldObject();
			switch (_type) {
				case red_deer:
				{

				}
					break;
				case tree:
				{
					b2BodyDef* bodyDef = new b2BodyDef();
					ob->body = bworld->CreateBody(bodyDef);

					b2FixtureDef* def = new b2FixtureDef();
					
				}
					break;
			}
		}


	};

}

#endif