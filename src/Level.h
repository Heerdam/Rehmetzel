#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	using namespace UI;

	struct Level : Ressource {
	protected:
		virtual void load() override {};
		virtual void unload() override {};
		virtual bool glLoad(void*) override {};
		virtual bool glUnload(void*) override {};
	public:
		Level(std::string);

		bool isActive = false;
		virtual void update() {};
		virtual void draw() {};
	};

	struct PreLoadLevel : public Level {
		PreLoadLevel() : Level("PreLoadLevel") {};
	};

	struct LoadingScreenLevel : public Level {
		LoadingScreenLevel() : Level("LoadingScreenLevel") {};

		Label* label;

		void load() override;
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

		TextureDebugRenderer* debug;
		GLuint* tex;

		Model* model, *floorModel;
		sf::Texture* mTex;
		
		View* view;

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

		void load() override;
		void update() override;
		void draw() override;
	};

	class LevelManager {

		std::unordered_map<std::string, Level*> levels;

		Level* activeLevel = nullptr;

	public:
		void initialize();

	public:
		void update();
		void draw();
	};

}