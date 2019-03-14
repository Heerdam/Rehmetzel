#pragma once

#include <Box2D/Box2D.h>
#include <unordered_map>

#include "MainStruct.hpp"

namespace Heerbann {

	using namespace Heerbann;

	class WorldBuilder;

	namespace BehaviourTree {
		struct Root;
	}
	
	using namespace BehaviourTree;

	enum EntityType {
		red_deer, tree, hunter
	};

	class World {
	public:

		WorldBuilder* builder;

		struct WorldObject {
			WorldObject* next = 0;
			WorldObject* tail = 0;

			bool isStatic = true;

			long id = Main::getId();
			EntityType type;
			b2Body* body;

			Root* root; //behaviour
		};

		b2World* bworld = new b2World(b2Vec2(0, -9.8f));
		std::unordered_map<long, WorldObject*> objects;

		void update(float _deltaTime){  
  			bworld->Step(_deltaTime, 8, 3);
		}

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

		void raycast(b2RayCastCallback* callback, const sf::Vector2f& _p1, const sf::Vector2f& _p2) {
			bworld->RayCast(callback, b2Vec2(_p1.x, _p1.y), b2Vec2(_p2.x, _p2.y));
		};

		void AABB(b2QueryCallback* callback, const sf::Vector2f& _p1, const sf::Vector2f& _p2) {
			b2AABB aabb;
			aabb.lowerBound.Set(_p1.x, _p1.y);
			aabb.upperBound.Set(_p2.x, _p2.y);
			bworld->QueryAABB(callback, aabb);
		}

		WorldObject* operator[](long _id) {
			if (objects.count(_id) == 0) return nullptr;
			return objects[_id];
		}

	};

	struct WorldBuilderDefinition {
		long seed = 0;
	};

	struct WorldOut {
		float** bgs;
		int vertexcount;
	};

	class WorldBuilder {
	public:
		//thread safe and stateless
		WorldOut* build(const WorldBuilderDefinition&);

	};

}