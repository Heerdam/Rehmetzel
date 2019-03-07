#pragma once

#include<vector>
#include<unordered_map>
#include<queue>
#include <thread>
#include <atomic>

#include "MainStruct.hpp"

namespace Heerbann {

	using namespace Heerbann;

	enum Type {
		texture
	};

	class AssetManager {

		struct LoadItem {
			Type type;
			std::string id;
			void* data;
			LoadItem(std::string _id, Type _type) : id(_id), type(_type) {};
		};

		std::queue<LoadItem*> loadQueue;
		std::queue<LoadItem*> unloadQueue;

		std::unordered_map<std::string, LoadItem*> assets;

		std::atomic<bool> isLoading = false;

		std::thread t1;

	public:

		LoadItem* operator[](std::string _id) {
			if (isLoading) std::exception("Assetmanager is currently loading. Nothing can be accessed");
			if (assets.count(_id) == 0) return nullptr;
			return assets[_id];
		};

		//enqueues a new asset to load
		//nothing can be added while loading
		void load(std::string _id, Type _type);

		//enqueues a new asset to unload
		//nothing can be added while loading
		void unload(std::string _id);

	private:
		void asyncLoad();

		std::atomic<float> progress;

	public:
		float update();

		bool loading() {
			return isLoading;
		};

		void finish();

	};

}
