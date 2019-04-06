#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	using namespace Heerbann;

	class WorldBuilder;
	class ShaderProgram;

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
		bool isVAO = false;

		const long long id = Main::getId();
		unsigned long lastSeen;

		EntityType type;
		b2Body* body;

		Root* root; //behaviour

		std::function<void(WorldObject*, float, sf::RenderWindow&)> draw;
		//finalize loading called on main thread
		std::function<void(WorldObject*)> finishedLoading;
	};

	class DebugDraw;

	class World {

		friend WorldBuilder;

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

	class BGVAO;
	class IndexedVAO;

	struct WorldOut {
		std::vector<BGVAO*> bgVAOs;
		std::vector<IndexedVAO*> indexVAOs;

		void finalize(ShaderProgram*, ShaderProgram*);
	};

	class WorldBuilder : public b2QueryCallback {
		
		BGVAO* createBGVAO(sf::Vector2f, int);
		IndexedVAO* createTrees(int, sf::Vector2f, sf::Vector2f);

	public:
		//thread safe and stateless
		WorldOut* build(const WorldBuilderDefinition&);
	};

}