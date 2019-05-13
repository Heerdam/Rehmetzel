#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

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
		virtual void load() {};

		//actual async unloading 
		virtual void unload() {};

		//add assets to queue for loading
		virtual void preLoad() {};

		//add assets to queue for unloading
		virtual void preUnload() {};

		//on main thread for opengl
		virtual void postLoad() {};
		
		virtual void update() {};
		virtual void draw() {};
	};

	struct PreLoadLevel : public Level {
		PreLoadLevel() : Level("PreLoadLevel") {};

		void preLoad() override;
		void postLoad() override;
	};

	struct LoadingScreenLevel : public Level {
		LoadingScreenLevel() : Level("LoadingScreenLevel") {};

		Label* label;

		void preLoad() override;
		void load() override;
		void preUnload() override;
		void postLoad() override;
		void unload() override;
		void update() override;
	};

	struct MainMenuLevel : public Level {
		MainMenuLevel() : Level("MainMenuLevel") {};

		void load() override;
		void unload() override;
		void update() override;
	};

	struct TestWorldLevel : public Level {
		TestWorldLevel() : Level("TestWorldLevel") {};

		ShaderProgram* bgShader;
		ShaderProgram* treeShader;
		ShaderProgram* sf_vsmShader;
		ShaderProgram* sf_vsmShaderNoTex;
		ShaderProgram* vsmShader;

		VSMRenderer* vsm;
		VSMRenderable* drawable_1;
		VSMRenderable* drawable_2;

		Model* model, *floorModel;
		sf::Texture* mTex;
		
		View* view;

		WorldOut* world;
		sLight* sun;
		Light* sl;
		float angle = 0;

		float* data;
		int vertexCount;

		AxisWidgetCamera* wCam;
		PerspectiveCamera* camera;
		bool buttonPressed = false;
		sf::Vector2i oldPos;
		float azimuth = 90.f;
		float altitude = 90.f;

		Text::TextBlock* testblock;

		void preLoad() override;
		void load() override;
		void postLoad() override;
		void update() override;
		void draw() override;
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
		void update();
		void draw();

		void queueLevelToLoad(std::string);
		void queueLevelToUnLoad(std::string);

		void queueLevelToLoad(Level*);
		void queueLevelToUnLoad(Level*);
	};

}