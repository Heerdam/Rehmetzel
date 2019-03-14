
#include "Level.h"
#include "Assets.hpp"
#include "UI.hpp"
#include <iostream>

using namespace Heerbann;
using namespace UI;

void LevelManager::initialize() {
	auto assets = Main::getAssetManager();
	assets->addLevel("LoadingScreenLevel", new LoadingScreenLevel());
	assets->addLevel("MainMenuLevel", new MainMenuLevel());
	loadLevel("LoadingScreenLevel");
	assets->finish();
};

void LevelManager::loadLevel(std::string _id) {
	auto assets = Main::getAssetManager();
	Level* level = assets->getLevel(_id);
	for (auto a : level->assetToLoad)
		assets->load(a->id);
	assets->loadLevel(_id);
	activeLevels.emplace_back(level);
}

void LevelManager::unloadLevel(std::string _id) {
	auto assets = Main::getAssetManager();
	Level* level = assets->getLevel(_id);
	for (auto a : level->assetToUnload)
		assets->unload(a->id);
	assets->unloadLevel(_id);
	for (auto it = activeLevels.begin(); it != activeLevels.end(); ++it)
		if (*it == level) activeLevels.erase(it);
}

void LevelManager::update(float _deltaTime) {
	for (auto l : activeLevels) {
		if (!l->isLocked && l->isLoaded)
			l->update(_deltaTime);
	}
}

void LevelManager::draw(float _deltaTime, sf::RenderWindow& _window) {
	for (auto l : activeLevels) {
		if (!l->isLocked && l->isLoaded)
			l->draw(_deltaTime, _window);
	}
}

//---------------------- LoadingScreenLevel ----------------------\\

Heerbann::LoadingScreenLevel::LoadingScreenLevel() {
	assetToLoad.emplace_back(new LoadItem("assets/tex/Forest_soil_diffuse.png",Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestCliff_basecolor.png", Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestDirt_diffuse.png", Type::texture));
	
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestGrass_basecolor.png", Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestMoss_basecolor.png", Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestMud_baseColor.png", Type::texture));
	
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestRoad_diffuse.png", Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestRock_basecolor.png", Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestWetMud_baseColor.png", Type::texture));

	assetToLoad.emplace_back(new LoadItem("assets/fonts/black.ttf", Type::font));
	//assetToLoad.emplace_back(new LoadItem("assets/shader/bg_shader", Type::shader));

	auto assets = Main::getAssetManager();
	for (auto a : assetToLoad)
		assets->addAsset(a);
}

void Heerbann::LoadingScreenLevel::load(AssetManager* _manager) {
	label = new Label("99%", (sf::Font*)((*_manager)["assets/fonts/black.ttf"]->data));
	label->position = sf::Vector2i(Main::width() / 2, Main::height() / 2);
	//Main::getStage()->add(label);

	bgShader = new sf::Shader();
	if (!bgShader->isAvailable()) std::exception("something wrong with bg shader");
	if (!bgShader->loadFromFile("assets/shader/bg_shader.vert", "assets/shader/bg_shader.geom", "assets/shader/bg_shader.frag"))
		std::exception("vertex/ geom/ fragment failed");

	

	//uniformlocation = glGetUniformLocation(bgShader->getNativeHandle(), "tex");

	tex[0] = (sf::Texture*)(Main::asset_getAsset("assets/tex/Forest_soil_diffuse.png")->data);
	tex[1] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestCliff_basecolor.png")->data);
	tex[2] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestDirt_diffuse.png")->data);

	tex[3] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestGrass_basecolor.png")->data);
	tex[4] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestMoss_basecolor.png")->data);
	tex[5] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestMud_baseColor.png")->data);

	tex[6] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestRoad_diffuse.png")->data);
	tex[7] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestRock_basecolor.png")->data);
	tex[8] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestWetMud_baseColor.png")->data);

	//TODO: create buffers

	pos = new float[2]{ 500, 0 };
	index = new unsigned char[1]{ 1 };

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo[2];
	glGenBuffers(2, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	/**
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(unsigned char), (void*)0);
	glEnableVertexAttribArray(1);
	*/
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);



}


void Heerbann::LoadingScreenLevel::unload(AssetManager* _manager) {

}

void Heerbann::LoadingScreenLevel::update(float _deltaTime) {

}

void Heerbann::LoadingScreenLevel::draw(float _deltaTime, sf::RenderWindow& _window) {
	_window.pushGLStates();
	//glUseProgram(bgShader->getNativeHandle());
	sf::Shader::bind(bgShader);

	/**
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex[0]->getNativeHandle());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex[1]->getNativeHandle());

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex[2]->getNativeHandle());

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, tex[3]->getNativeHandle());

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, tex[4]->getNativeHandle());

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, tex[5]->getNativeHandle());

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, tex[6]->getNativeHandle());

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, tex[7]->getNativeHandle());

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, tex[8]->getNativeHandle());
	*/
	
	//const GLint samplers[9]{ 0, 1, 2, 3, 4, 5, 6, 7, 8 };
	//glUniform1iv(uniformlocation, 9, samplers);

	glBindVertexArray(vao);
	//glDrawArrays(GL_POINTS, 0, 1);

	glBindVertexArray(0);

	glUseProgram(0);

	_window.popGLStates();
}

//---------------------- MainMenuLevel ----------------------\\

Heerbann::MainMenuLevel::MainMenuLevel() {

}

void Heerbann::MainMenuLevel::load(AssetManager* _manager) {

}

void Heerbann::MainMenuLevel::unload(AssetManager* _manager) {

}

void Heerbann::MainMenuLevel::update(float _deltaTime) {

}

void Heerbann::MainMenuLevel::draw(float _deltaTime, sf::RenderWindow& _window) {

}

