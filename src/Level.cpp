
#include "Level.h"
#include "Assets.hpp"
#include "UI.hpp"
#include "CameraUtils.hpp"
#include "Utils.hpp"
#include "World.hpp"
#include "TextUtil.hpp"
#include "AI.hpp"
#include "G3D.hpp"
#include "InputMultiplexer.hpp"

using namespace Heerbann;
using namespace UI;

Level::Level(std::string _id) : id(_id) {
	neededLoadingState = State::discrete;
}

void LevelManager::initialize() {
	M_Asset->addLevel("PreLoadLevel", new PreLoadLevel());
	M_Asset->addLevel("LoadingScreenLevel", new LoadingScreenLevel());
	M_Asset->addLevel("MainMenuLevel", new MainMenuLevel());
	M_Asset->addLevel("TestWorldLevel", new TestWorldLevel());

	queueLevelToLoad("PreLoadLevel");
};

void LevelManager::loadLevel(Level* _level) {
	_level->preLoad(M_Asset);
	for (auto a : _level->assetToLoad) {
		if (!M_Asset->exists(a->id))
			M_Asset->addAsset(a);
		M_Asset->load(a->id);
	}		
	for (auto a : _level->assetToUnload) {
		if (M_Asset->exists(a->id))
			M_Asset->unload(a->id);
	}
	M_Asset->loadLevel(_level->id);
	if (_level->neededLoadingState == State::discrete && _level->lockIfDiscrete)
		M_Asset->finish();
	_level->assetToLoad.clear();
	_level->assetToUnload.clear();
	activeLevels.emplace_back(_level);
	_level->postLoad(M_Asset);
}

void LevelManager::unloadLevel(Level* _level) {
	_level->preUnload(M_Asset);
	for (auto a : _level->assetToLoad) {
		if (!M_Asset->exists(a->id))
			M_Asset->addAsset(a);
		M_Asset->load(a->id);
	}
	for (auto a : _level->assetToUnload) {
		if (M_Asset->exists(a->id))
			M_Asset->unload(a->id);
	}
	M_Asset->unloadLevel(_level->id);
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

void LevelManager::draw(float _deltaTime, SpriteBatch* _batch) {
	for (auto l : activeLevels) {
		if (!l->isLocked && l->isLoaded)
			l->draw(_deltaTime, _batch);
	}
}

void LevelManager::queueLevelToLoad(std::string _id) {
	Level* level = M_Asset->getLevel(_id);
	queueLevelToLoad(level);
}

void LevelManager::queueLevelToUnLoad(std::string _id) {
	Level* level = M_Asset->getLevel(_id);
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
	//assetToLoad.emplace_back(new LoadItem("assets/tex/Forest_soil_diffuse.png", Type::texture_png));
	//assetToLoad.emplace_back(new LoadItem("assets/tex/ForestCliff_basecolor.png", Type::texture_png));
	//assetToLoad.emplace_back(new LoadItem("assets/tex/ForestDirt_diffuse.png", Type::texture_png));

	//assetToLoad.emplace_back(new LoadItem("assets/tex/ForestGrass_basecolor.png", Type::texture_png));
	//assetToLoad.emplace_back(new LoadItem("assets/tex/ForestMoss_basecolor.png", Type::texture_png));
	//assetToLoad.emplace_back(new LoadItem("assets/tex/ForestMud_baseColor.png", Type::texture_png));

	//assetToLoad.emplace_back(new LoadItem("assets/tex/ForestRoad_diffuse.png", Type::texture_png));
	//assetToLoad.emplace_back(new LoadItem("assets/tex/ForestRock_basecolor.png", Type::texture_png));
	//assetToLoad.emplace_back(new LoadItem("assets/tex/ForestWetMud_baseColor.png", Type::texture_png));

	//assetToLoad.emplace_back(new LoadItem("assets/shader/bg_shader", Type::shader));

	//assetToLoad.emplace_back(new LoadItem("assets/shader/bg_shader", Type::shader));
	//assetToLoad.emplace_back(new LoadItem("assets/shader/tree_shader", Type::shader));
	//assetToLoad.emplace_back(new LoadItem("assets/shader/model_shader", Type::shader));
	//assetToLoad.emplace_back(new LoadItem("assets/shader/AiTest", Type::shader));
	//assetToLoad.emplace_back(new LoadItem("assets/shader/AiTestComp", Type::shader));

	//assetToLoad.emplace_back(new LoadItem("assets/trees/trees", Type::atlas));

	assetToLoad.emplace_back(new LoadItem("assets/3d/deer/Model/Deer_old.dae", Type::model));
	assetToLoad.emplace_back(new LoadItem("assets/shader/simple forward/sb_sf", Type::shader));
	assetToLoad.emplace_back(new LoadItem("assets/shader/spritebatch/sf_font", Type::shader));
}

void PreLoadLevel::postLoad(AssetManager* _assets) {
	M_Level->queueLevelToUnLoad(this);
	M_Level->queueLevelToLoad("LoadingScreenLevel");
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
	M_Level->queueLevelToUnLoad(this);
	M_Level->queueLevelToLoad("TestWorldLevel");
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
	//WorldBuilderDefinition def;
	//world = Main::getWorld()->builder->build(def);	

}

void TestWorldLevel::postLoad(AssetManager* _asset) {	
	//world->finalize(bgShader, treeShader);
	//Main::getAI()->create();
	//bgShader = reinterpret_cast<ShaderProgram*>(Main::getAssetManager()->getAsset("assets/shader/bg_shader")->data);
	//treeShader = reinterpret_cast<ShaderProgram*>(Main::getAssetManager()->getAsset("assets/shader/tree_shader")->data);
	//TODO
}

void TestWorldLevel::update(float _delta) {
	
}

void TestWorldLevel::draw(float _delta, SpriteBatch* _batch) {
	//for (auto v : world->bgVAOs)
		//v->draw(bgShader);
	//for (auto v : world->indexVAOs)
		//v->draw(treeShader);
	//Main::getAI()->draw(Main::getViewport()->cam.getTransform());
}

