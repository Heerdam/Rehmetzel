#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	using namespace Heerbann;

	enum EntityType {
		red_deer, tree, hunter
	};

	struct WorldObject {
		WorldObject* next = 0;
		WorldObject* tail = 0;

		bool isStatic = true;
		bool isLoaded = false;
		bool isVAO = false;

		const long long id = ID;
		unsigned long lastSeen;

		EntityType type;

		std::function<void(WorldObject*, float, sf::RenderWindow&)> draw;
		//finalize loading called on main thread
		std::function<void(WorldObject*)> finishedLoading;
	};

	class World {

		friend WorldBuilder;

		DebugDraw* debug;

		std::unordered_map<long long, WorldObject*> objects;

		std::queue<WorldObject*> finishQueue;

		std::mutex mapLock;
		std::mutex worldLock;

	public:

		World();
		WorldBuilder* builder;
		void update(float);
		void debugDraw();
		long long create(EntityType, Vec2);

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
		std::vector<BGVAO*> bgVAOs;
		std::vector<IndexedVAO*> indexVAOs;

		//void finalize(ShaderProgram*, ShaderProgram*);
	};

}