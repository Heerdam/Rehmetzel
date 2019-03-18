
#include "Level.h"
#include "Assets.hpp"
#include "UI.hpp"
#include "CameraUtils.hpp"
#include "Utils.hpp"
#include "World.hpp"

#include <iostream>

using namespace Heerbann;
using namespace UI;

Level::Level(std::string _id) : id(_id) {
	neededLoadingState = State::discrete;
}

void LevelManager::initialize() {
	auto assets = Main::getAssetManager();
	assets->addLevel("PreLoadLevel", new PreLoadLevel());
	assets->addLevel("LoadingScreenLevel", new LoadingScreenLevel());
	assets->addLevel("MainMenuLevel", new MainMenuLevel());
	assets->addLevel("TestWorldLevel", new TestWorldLevel());

	queueLevelToLoad("PreLoadLevel");
};

void LevelManager::loadLevel(Level* _level) {
	auto assets = Main::getAssetManager();
	_level->preLoad(assets);
	for (auto a : _level->assetToLoad) {
		if (!assets->exists(a->id))
			assets->addAsset(a);
		assets->load(a->id);
	}		
	for (auto a : _level->assetToUnload) {
		if (assets->exists(a->id))
			assets->unload(a->id);
	}
	assets->loadLevel(_level->id);
	if (_level->neededLoadingState == State::discrete && _level->lockIfDiscrete)
		assets->finish();	
	_level->assetToLoad.clear();
	_level->assetToUnload.clear();
	activeLevels.emplace_back(_level);
	_level->postLoad(assets);
}

void LevelManager::unloadLevel(Level* _level) {
	auto assets = Main::getAssetManager();
	_level->preUnload(assets);
	for (auto a : _level->assetToLoad) {
		if (!assets->exists(a->id))
			assets->addAsset(a);
		assets->load(a->id);
	}
	for (auto a : _level->assetToUnload) {
		if (assets->exists(a->id))
			assets->unload(a->id);
	}
	assets->unloadLevel(_level->id);
	_level->assetToLoad.clear();
	_level->assetToUnload.clear();
	for (auto it = activeLevels.begin(); it != activeLevels.end(); ++it)
		if (*it == _level) {
			activeLevels.erase(it);
			break;
		}
}

void LevelManager::update(float _deltaTime) {
	//this is used so that loading isnt caught in a loop
	if (!loadCache.empty()) {
		for (Level* l : loadCache)
			toLoad.emplace(l);
		loadCache.clear();
	}
	if (!unloadCache.empty()) {
		for (Level* l : unloadCache)
			toUnload.emplace(l);
		unloadCache.clear();
	}
	while (!toUnload.empty()) {
		Level* next = toUnload.front();
		toUnload.pop();
		unloadLevel(next);
	}
	while (!toLoad.empty()) {
		Level* next = toLoad.front();
		toLoad.pop();
		loadLevel(next);
	}
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

void LevelManager::queueLevelToLoad(std::string _id) {
	auto assets = Main::getAssetManager();
	Level* level = assets->getLevel(_id);
	queueLevelToLoad(level);
}

void LevelManager::queueLevelToUnLoad(std::string _id) {
	auto assets = Main::getAssetManager();
	Level* level = assets->getLevel(_id);
	queueLevelToUnLoad(level);
}

void LevelManager::queueLevelToLoad(Level* _level) {
	loadCache.emplace_back(_level);
}

void LevelManager::queueLevelToUnLoad(Level* _level) {
	unloadCache.emplace_back(_level);
}

//---------------------- PreLoadLevel ----------------------\\

void PreLoadLevel::preLoad(AssetManager* _asset) {
	//assetToLoad.emplace_back(new LoadItem("assets/fonts/black.ttf", Type::font));
	assetToLoad.emplace_back(new LoadItem("assets/tex/Forest_soil_diffuse.png", Type::texture_png));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestCliff_basecolor.png", Type::texture_png));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestDirt_diffuse.png", Type::texture_png));

	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestGrass_basecolor.png", Type::texture_png));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestMoss_basecolor.png", Type::texture_png));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestMud_baseColor.png", Type::texture_png));

	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestRoad_diffuse.png", Type::texture_png));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestRock_basecolor.png", Type::texture_png));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestWetMud_baseColor.png", Type::texture_png));

	assetToLoad.emplace_back(new LoadItem("assets/shader/bg_shader", Type::shader));
	assetToLoad.emplace_back(new LoadItem("assets/shader/tree_shader", Type::shader));

	//assetToLoad.emplace_back(new LoadItem("assets/trees/poplar_07_top.png", Type::texture));

	assetToLoad.emplace_back(new LoadItem("assets/trees/trees", Type::atlas));
}

void PreLoadLevel::postLoad(AssetManager* _assets) {
	Main::getLevel()->queueLevelToUnLoad(this);
	Main::getLevel()->queueLevelToLoad("LoadingScreenLevel");
}

//---------------------- LoadingScreenLevel ----------------------\\

void LoadingScreenLevel::preLoad(AssetManager *) {
	
}

void LoadingScreenLevel::load(AssetManager* _manager) {
	//label = new Label("99%", (sf::Font*)((*_manager)["assets/fonts/black.ttf"]->data));
	//label->position = sf::Vector2i(Main::width() / 2, Main::height() / 2);
	//Main::getStage()->add(label);

}

void LoadingScreenLevel::preUnload(AssetManager* _manager) {

}

void LoadingScreenLevel::postLoad(AssetManager* _asset) {
	Main::getLevel()->queueLevelToUnLoad(this);
	Main::getLevel()->queueLevelToLoad("TestWorldLevel");
}

void LoadingScreenLevel::unload(AssetManager* _manager) {
	
}

void LoadingScreenLevel::update(float _deltaTime) {

}

//---------------------- MainMenuLevel ----------------------\\

void MainMenuLevel::load(AssetManager* _manager) {

}

void MainMenuLevel::unload(AssetManager* _manager) {

}

void MainMenuLevel::update(float _deltaTime) {

}

//---------------------- TestWorldLevel ----------------------\\

void TestWorldLevel::preLoad(AssetManager *) {
	
	
}

void TestWorldLevel::load(AssetManager* _asset) {
	
	WorldBuilderDefinition def;

	world = Main::getWorld()->builder->build(def);
	
	bgShader = (sf::Shader*)Main::getAssetManager()->getAsset("assets/shader/bg_shader");
	treeShader = (sf::Shader*)Main::getAssetManager()->getAsset("assets/shader/tree_shader");
}

void TestWorldLevel::postLoad(AssetManager* _asset) {	

	world->finalize(bgShader, treeShader);
}

void TestWorldLevel::update(float _delta) {
}

void TestWorldLevel::draw(float _delta, sf::RenderWindow& _window) {
	for (auto v : world->bgVAOs)
		v->draw(bgShader);
	for (auto v : world->indexVAOs)
		v->draw(treeShader);
}

