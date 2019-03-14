#pragma once

#include <vector>
#include <atomic>
#include <queue>

#include "MainStruct.hpp"

namespace Heerbann {

	enum State : int;

	namespace UI {
		class Label;
	}

	using namespace UI;

	//base class to be inherited
	struct Level {
	public:
		const std::string id;
		Level(std::string);
		std::atomic<bool> isLocked = false;
		std::atomic<bool> isLoaded = false;

		std::vector<LoadItem*> assetToLoad;
		std::vector<LoadItem*> assetToUnload;

		State neededLoadingState;
		bool lockIfDiscrete = true;

		virtual void load(AssetManager*) {};
		virtual void unload(AssetManager*) {};

		virtual void preLoad(AssetManager*) {};
		//on main thread for opengl
		virtual void postLoad(AssetManager*) {};
		
		virtual void preUnload(AssetManager*) {};

		virtual void update(float) {};
		virtual void draw(float, sf::RenderWindow&) {};
	};

	struct PreLoadLevel : public Level {
		PreLoadLevel() : Level("PreLoadLevel") {};

		void preLoad(AssetManager*) override;
		void postLoad(AssetManager*) override;
	};

	struct LoadingScreenLevel : public Level {
		LoadingScreenLevel() : Level("LoadingScreenLevel") {};

		Label* label;

		void preLoad(AssetManager*) override;
		void load(AssetManager*) override;
		void preUnload(AssetManager*) override;
		void postLoad(AssetManager*) override;
		void unload(AssetManager*) override;
		void update(float) override;
	};

	struct MainMenuLevel : public Level {
		MainMenuLevel() : Level("MainMenuLevel") {};

		void load(AssetManager*) override;
		void unload(AssetManager*) override;
		void update(float) override;
	};

	struct TestWorldLevel : public Level {
		TestWorldLevel() : Level("TestWorldLevel") {};

		sf::Shader* bgShader;

		//background
		int vertexCount = 9;

		GLuint vao, vbo;
		GLint cameraUniformHandle;
		GLuint texLoc[9];
		sf::Texture* tex[9];

		float* pos;

		void preLoad(AssetManager*) override;
		void load(AssetManager*) override;
		void postLoad(AssetManager*) override;
		void update(float) override;
		void draw(float, sf::RenderWindow&) override;
	};

	class LevelManager {

		std::vector<Level*> activeLevels;

	public:

		void initialize();

	private:
		std::queue<Level*> toLoad;
		std::queue<Level*> toUnload;

		void loadLevel(Level*);
		void unloadLevel(Level*);

	public:
		void update(float);
		void draw(float, sf::RenderWindow&);

		void queueLevelToLoad(std::string);
		void queueLevelToUnLoad(std::string);

		void queueLevelToLoad(Level*);
		void queueLevelToUnLoad(Level*);
	};

}