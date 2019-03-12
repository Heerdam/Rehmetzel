#pragma once

#include <vector>
#include <unordered_map>
#include <queue>
#include <thread>
#include <tuple>
#include <mutex>

#include "MainStruct.hpp"
#include "Assets.hpp"

namespace Heerbann {

	struct Level {
		std::atomic<bool> isLocked = false;
		bool isLoaded = false;

		std::vector <std::tuple<std::string, Type>> assetLoadQueue;
		std::vector <std::tuple<std::string, Type>> assetUnloadQueue;
	};

	class LevelLoader {

		std::unordered_map<std::string, Level*> levels;

		std::queue<Level*> loadingQueue;
		std::queue<Level*> unloadingQueue;

		std::mutex loadlock;
		std::mutex unloadlock;

		Level* popLoad() {						
			std::unique_lock<std::mutex> guard(loadlock);
			guard.lock();
			Level* next = loadingQueue.front();
			loadingQueue.pop();
			guard.unlock();
			return next;
		};

		void enqueueLoad(Level* _level) {
			std::unique_lock<std::mutex> guard(loadlock);
			guard.lock();
			loadingQueue.emplace(_level);
			guard.unlock();
		};

		Level* popUnload() {
			std::unique_lock<std::mutex> guard(unloadlock);
			guard.lock();
			Level* next = unloadingQueue.front();
			loadingQueue.pop();
			guard.unlock();
			return next;
		};

		void enqueueUnload(Level* _level) {
			std::unique_lock<std::mutex> guard(unloadlock);
			guard.lock();
			unloadingQueue.emplace(_level);
			guard.unlock();
		};

		void asyncLoad(Level* _toLoad) {

		};

	public:

		void addLevel(std::string _id, Level* _level) {

		};

		//adds a level to the loading queue (FIFO)
		void loadLevel(std::string _id) {

		};

		//adds a level to the unloading queue (FIFO)
		void unloadLevel(std::string _id) {

		};

	};





}