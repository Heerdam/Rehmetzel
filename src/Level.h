#pragma once

#include <vector>
#include <unordered_map>
#include <queue>
#include <thread>
#include <tuple>
#include <mutex>
#include <functional>

#include "MainStruct.hpp"

namespace Heerbann {

	//base class to be inherited
	struct Level {
		std::atomic<bool> isLocked = false;
		bool isLoaded = false;

		std::function<void(AssetManager*)> load;
		std::function<void(AssetManager*)> unload;
	};

}