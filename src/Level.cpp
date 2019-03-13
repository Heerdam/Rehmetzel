
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
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestGrass_basecolor.png",Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/fonts/black.ttf", Type::font));

	auto assets = Main::getAssetManager();
	for (auto a : assetToLoad)
		assets->addAsset(a);
}

void Heerbann::LoadingScreenLevel::load(AssetManager* _manager) {
	label = new Label("0%", (sf::Font*)((*_manager)["assets/fonts/black.ttf"]->data));
	label->position = sf::Vector2i(Main::width() / 2, Main::height() / 2);
	Main::getStage()->add(label);
}

void Heerbann::LoadingScreenLevel::unload(AssetManager* _manager) {

}

void Heerbann::LoadingScreenLevel::update(float _deltaTime) {

}

void Heerbann::LoadingScreenLevel::draw(float _deltaTime, sf::RenderWindow& _window) {

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

