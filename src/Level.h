#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	enum State : int;

	class ShaderProgram;
	class Matrix4;

	namespace UI {
		class Label;
		class StaticLabel;
	}

	namespace G3D {
		struct Model;
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

		//actual async loading 
		virtual void load(AssetManager*) {};

		//actual async unloading 
		virtual void unload(AssetManager*) {};

		//add assets to queue for loading
		virtual void preLoad(AssetManager*) {};

		//add assets to queue for unloading
		virtual void preUnload(AssetManager*) {};

		//on main thread for opengl
		virtual void postLoad(AssetManager*) {};
		
		virtual void update(float) {};
		virtual void draw(float, SpriteBatch*) {};
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

	class BGVAO;

	struct WorldOut;

	namespace Text {
		class TextBlock;
	}

	class PerspectiveCamera;
	class AxisWidgetCamera;
	
	struct TestWorldLevel : public Level {
		TestWorldLevel() : Level("TestWorldLevel") {};

		ShaderProgram* bgShader;
		ShaderProgram* treeShader;
		ShaderProgram* modelShader;

		G3D::Model* model;
		GLuint camPos;

		WorldOut* world;

		float* data;
		int vertexCount;

		AxisWidgetCamera* wCam;
		PerspectiveCamera* camera;
		bool buttonPressed = false;
		sf::Vector2i oldPos;
		float azimuth = 90.f;
		float altitude = 90.f;

		Text::TextBlock* testblock;

		StaticLabel* label;

		void preLoad(AssetManager*) override;
		void load(AssetManager*) override;
		void postLoad(AssetManager*) override;
		void update(float) override;
		void draw(float, SpriteBatch*) override;
	};

	class LevelManager {

		std::vector<Level*> activeLevels;

	public:
		void initialize();

	private:
		std::queue<Level*> toLoad;
		std::queue<Level*> toUnload;

		std::vector<Level*> loadCache;
		std::vector<Level*> unloadCache;

		void loadLevel(Level*);
		void unloadLevel(Level*);

	public:
		void update(float);
		void draw(float, SpriteBatch*);

		void queueLevelToLoad(std::string);
		void queueLevelToUnLoad(std::string);

		void queueLevelToLoad(Level*);
		void queueLevelToUnLoad(Level*);
	};

}