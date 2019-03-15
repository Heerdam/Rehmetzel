#pragma once

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

	struct WorldObject {
		WorldObject* next = 0;
		WorldObject* tail = 0;

		bool isStatic = true;
		bool isLoaded = false;

		const long id = Main::getId();
		unsigned long lastSeen;

		EntityType type;
		b2Body* body;

		sf::Sprite* sprite;

		Root* root; //behaviour

		std::function<void(WorldObject*, float, sf::RenderWindow&)> draw;
		//finalize loading called on main thread
		std::function<void(WorldObject*)> finishedLoading;
	};

	class DebugDraw;

	class World {

		DebugDraw* debug;

		b2World* bworld = new b2World(b2Vec2(0, -9.8f));
		std::unordered_map<long, WorldObject*> objects;

		std::queue<WorldObject*> finishQueue;

		std::mutex mapLock;
		std::mutex worldLock;

	public:

		World();
		WorldBuilder* builder;
		void update(float);
		void debugDraw();
		long create(EntityType, sf::Vector2f);
		void raycast(b2RayCastCallback*, const sf::Vector2f&, const sf::Vector2f&);
		void AABB(b2QueryCallback*, const sf::Vector2f&, const sf::Vector2f&);

		WorldObject* operator[](long _id) {
			std::lock_guard<std::mutex> guard(mapLock);
			if (objects.count(_id) == 0) return nullptr;
			return objects[_id];
		};

	};

	struct WorldBuilderDefinition {
		long seed = 0;
	};

	struct WorldOut {
		float** bgs;
		int vertexcount;
	};

	class WorldBuilder : public b2QueryCallback {
	public:
		//thread safe and stateless
		WorldOut* build(const WorldBuilderDefinition&);
	};

}