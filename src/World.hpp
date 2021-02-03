#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	using namespace Heerbann;

	
	struct WorldObject {
		const std::string id;
		void* object;

		template<class T>
		T get() {
			return reinterpret_cast<T>(object);
		};
	};



	struct WorldBuilderDefinition {

		uint heightMapWidth = 1000;
	};

	class World {

		std::unordered_map<std::string, WorldObject*> objects;

		VSMRenderer* renderer;

	public:
		World();
		void build(const WorldBuilderDefinition&);

		template<class T>
		WorldObject* get(std::string _id) {
			if (objects.count(_id) == 0) return nullptr;
			return reinterpret_cast<T>(objects[_id]->object);
		};
	};

}