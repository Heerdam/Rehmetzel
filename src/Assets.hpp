#pragma once

#include<vector>
#include<unordered_map>
#include<queue>
#include <thread>
#include <atomic>
#include <mutex>

#include "MainStruct.hpp"

namespace Heerbann {

	using namespace Heerbann;

	enum Type {
		texture, font, shader, level
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

		std::mutex loadQueueLock;
		std::mutex unloadQueueLock;
		std::mutex accessLock;

		std::unordered_map<std::string, LoadItem*> assets;

		std::atomic<bool> isLoading = false;
		std::atomic<bool> continousLoading = false;

		std::thread t1;

		LoadItem* popLoad();
		LoadItem* popUnload();

		void queueLoad(LoadItem*);
		void queueUnLoad(LoadItem*);

	public:

		//thread safe
		LoadItem* operator[](std::string _id) {
			std::unique_lock<std::mutex> guard(accessLock);
			guard.lock();
			if (assets.count(_id) == 0) return nullptr;
			auto asset = assets[_id];
			guard.unlock();
			return asset;
		};

		//enqueues a new asset to load
		void load(std::string _id, Type _type);

		//enqueues a new asset to unload
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
