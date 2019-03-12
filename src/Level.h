#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	//base class to be inherited
	struct Level {
		std::atomic<bool> isLocked = false;
		bool isLoaded = false;

		virtual void load(AssetManager*) = 0;
		virtual void unload(AssetManager*) = 0;

		virtual void update(float) = 0;
		virtual void draw(float, sf::RenderWindow&) = 0;
	};

}