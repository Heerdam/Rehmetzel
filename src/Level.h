#pragma once

#include <vector>
#include <atomic>

#include "Assets.hpp" //beware!

#include "MainStruct.hpp"

namespace Heerbann {

	//forward declaration doesnt work because of ambiguity 
	//class AssetManager {
	//public:
		//struct LoadItem;
	//};

	//base class to be inherited
	struct Level {
	public:
		std::atomic<bool> isLocked = false;
		std::atomic<bool> isLoaded = false;

		std::vector<AssetManager::LoadItem*> assetToLoad;
		std::vector<AssetManager::LoadItem*> assetToUnload;

		virtual void load(AssetManager*) = 0;
		virtual void unload(AssetManager*) = 0;

		virtual void update(float) = 0;
		virtual void draw(float, sf::RenderWindow&) = 0;
	};

	struct LoadingScreenLevel : public Level {
		void load(AssetManager*);
		void unload(AssetManager*);
		void update(float);
		void draw(float, sf::RenderWindow&);
	};

	struct MainMenuLevel : public Level {
		void load(AssetManager*);
		void unload(AssetManager*);
		void update(float);
		void draw(float, sf::RenderWindow&);
	};

	class LevelManager {

		std::vector<Level*> activeLevels;

	public:
		LevelManager();

		void loadLevel(std::string);
		void unloadLevel(std::string);

		void update(float);
		void draw(float, sf::RenderWindow&);
	};

}