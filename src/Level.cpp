
#include "Level.h"
#include "Assets.hpp"
#include "UI.hpp"

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
}

void LevelManager::unloadLevel(std::string _id) {
	auto assets = Main::getAssetManager();
	Level* level = assets->getLevel(_id);
	for (auto a : level->assetToUnload)
		assets->unload(a->id);
	assets->unloadLevel(_id);
}

void LevelManager::update(float _deltaTime) {
	for (auto l : activeLevels) {
		if (!l->isLocked && !l->isLoaded)
			l->update(_deltaTime);
	}
}

void LevelManager::draw(float _deltaTime, sf::RenderWindow& _window) {
	for (auto l : activeLevels) {
		if (!l->isLocked && !l->isLoaded)
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
	assetToLoad.emplace_back(new LoadItem("assets/shader/bg_shader", Type::shader));

	auto assets = Main::getAssetManager();
	for (auto a : assetToLoad)
		assets->addAsset(a);
}

void Heerbann::LoadingScreenLevel::load(AssetManager* _manager) {
	label = new Label("99%", (sf::Font*)((*_manager)["assets/fonts/black.ttf"]->data));
	label->position = sf::Vector2i(Main::width() / 2, Main::height() / 2);
	//Main::getStage()->add(label);

	bgShader = (sf::Shader*)(*_manager)["assets/shader/bg_shader"]->data;

	uniformlocation = glGetUniformLocation(bgShader->getNativeHandle(), "tex");

	tex[0] = (sf::Texture*)(Main::asset_getAsset("assets/tex/Forest_soil_diffuse.png")->data);
	tex[0] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestCliff_basecolor.png")->data);
	tex[0] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestDirt_diffuse.png")->data);

	tex[0] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestGrass_basecolor.png")->data);
	tex[0] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestMoss_basecolor.png")->data);
	tex[0] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestMud_baseColor.png")->data);

	tex[0] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestRoad_diffuse.png")->data);
	tex[0] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestRock_basecolor.png")->data);
	tex[0] = (sf::Texture*)(Main::asset_getAsset("assets/tex/ForestWetMud_baseColor.png")->data);

	//TODO: create buffers

	glGenBuffers(1, &posBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(0);

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(0);
}

void Heerbann::LoadingScreenLevel::unload(AssetManager* _manager) {

}

void Heerbann::LoadingScreenLevel::update(float _deltaTime) {

}

void Heerbann::LoadingScreenLevel::draw(float _deltaTime, sf::RenderWindow& _window) {
	sf::Shader::bind(bgShader);
	
	for (int i = 0; i < 9; ++i) {
		glActiveTexture(GL_TEXTURE + i);
		glBindTexture(GL_TEXTURE_2D, tex[i]->getNativeHandle());
	}	
	const GLint samplers[9]{ 0, 1, 2, 3, 4, 5, 6, 7, 8 };
	glUniform1iv(uniformlocation, 9, samplers);

	glBindVertexArray(posBuffer);
	glBindVertexArray(indexBuffer);
	glDrawArrays(GL_POINTS, 0, sizeof(pos) / 2);

	sf::Shader::bind(nullptr);
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

