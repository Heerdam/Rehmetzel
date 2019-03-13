#pragma once

#include <vector>
#include <atomic>

#include "MainStruct.hpp"

namespace Heerbann {

	namespace UI {
		class Label;
	}

	using namespace UI;

	//base class to be inherited
	struct Level {
	public:
		std::atomic<bool> isLocked = false;
		std::atomic<bool> isLoaded = false;

		std::vector<LoadItem*> assetToLoad;
		std::vector<LoadItem*> assetToUnload;

		virtual void load(AssetManager*) = 0;
		virtual void unload(AssetManager*) = 0;

		virtual void update(float) = 0;
		virtual void draw(float, sf::RenderWindow&) = 0;
	};

	struct LoadingScreenLevel : public Level {

		Label* label;

		sf::Shader* bgShader;

		GLint uniformlocation;
		GLuint posBuffer, indexBuffer;
		sf::Texture* tex[9];

		float* pos;
		unsigned char* index;

		LoadingScreenLevel();
		void load(AssetManager*);
		void unload(AssetManager*);
		void update(float);
		void draw(float, sf::RenderWindow&);
	};

	struct MainMenuLevel : public Level {
		MainMenuLevel();
		void load(AssetManager*);
		void unload(AssetManager*);
		void update(float);
		void draw(float, sf::RenderWindow&);
	};

	class LevelManager {

		std::vector<Level*> activeLevels;

	public:

		void initialize();

		void loadLevel(std::string);
		void unloadLevel(std::string);

		void update(float);
		void draw(float, sf::RenderWindow&);
	};

}