
#include "Level.h"
#include "Assets.hpp"

using namespace Heerbann;

Heerbann::LevelManager::LevelManager() {
	auto assets = MainStruct::get()->assets;
	assets->addLevel("LoadingScreenLevel", new LoadingScreenLevel());
	assets->addLevel("MainMenuLevel", new MainMenuLevel());
}

void Heerbann::LevelManager::loadLevel(std::string _id) {
	auto assets = MainStruct::get()->assets;
	Level* level = assets->getLevel(_id);
	for (auto a : level->assetToLoad)
		assets->loadLevel(a->id);
	assets->loadLevel(_id);
}

void Heerbann::LevelManager::unloadLevel(std::string _id) {
	auto assets = MainStruct::get()->assets;
	Level* level = assets->getLevel(_id);
	for (auto a : level->assetToUnload)
		assets->unloadLevel(a->id);
	assets->unloadLevel(_id);
}

void Heerbann::LevelManager::update(float _deltaTime) {
	for (auto l : activeLevels) {
		if (!l->isLocked && !l->isLoaded)
			l->update(_deltaTime);
	}
}

void Heerbann::LevelManager::draw(float _deltaTime, sf::RenderWindow& _window) {
	for (auto l : activeLevels) {
		if (!l->isLocked && !l->isLoaded)
			l->draw(_deltaTime, _window);
	}
}

void Heerbann::MainMenuLevel::load(AssetManager* _manager) {

}

void Heerbann::MainMenuLevel::unload(AssetManager* _manager) {

}

void Heerbann::MainMenuLevel::update(float _deltaTime) {

}

void Heerbann::MainMenuLevel::draw(float _deltaTime, sf::RenderWindow& _window) {

}

void Heerbann::LoadingScreenLevel::load(AssetManager* _manager) {

}

void Heerbann::LoadingScreenLevel::unload(AssetManager* _manager) {

}

void Heerbann::LoadingScreenLevel::update(float _deltaTime) {

}

void Heerbann::LoadingScreenLevel::draw(float _deltaTime, sf::RenderWindow& _window) {

}
