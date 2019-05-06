
#include "Level.h"
#include "Assets.hpp"
#include "UI.hpp"
#include "CameraUtils.hpp"
#include "Utils.hpp"
#include "World.hpp"
#include "TextUtil.hpp"
#include "AI.hpp"
#include "InputMultiplexer.hpp"
#include "Gdx.hpp"

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
	_level->preLoad();
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
	_level->postLoad();
}

void LevelManager::unloadLevel(Level* _level) {
	_level->preUnload();
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

void LevelManager::update() {
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
			l->update();
	}
}

void LevelManager::draw() {
	for (auto l : activeLevels) {
		if (!l->isLocked && l->isLoaded)
			l->draw();
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

void PreLoadLevel::preLoad() {
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
	assetToLoad.emplace_back(new LoadItem("assets/3d/deer/Textures/Deer Common.png", Type::texture_png));
}

void PreLoadLevel::postLoad() {
	M_Level->queueLevelToUnLoad(this);
	M_Level->queueLevelToLoad("LoadingScreenLevel");
}

//---------------------- LoadingScreenLevel ----------------------\\

void LoadingScreenLevel::preLoad() {
	
}

void LoadingScreenLevel::load() {
	//label = new Label("99%", (sf::Font*)((*_manager)["assets/fonts/black.ttf"]->data));
	//label->position = sf::Vector2i(Main::width() / 2, Main::height() / 2);
	//Main::getStage()->add(label);

}

void LoadingScreenLevel::preUnload() {

}

void LoadingScreenLevel::postLoad() {
	M_Level->queueLevelToUnLoad(this);
	M_Level->queueLevelToLoad("TestWorldLevel");
}

void LoadingScreenLevel::unload() {
	
}

void LoadingScreenLevel::update() {

}

//---------------------- MainMenuLevel ----------------------\\

void MainMenuLevel::load() {

}

void MainMenuLevel::unload() {

}

void MainMenuLevel::update() {

}

//---------------------- TestWorldLevel ----------------------\\

void TestWorldLevel::preLoad() {
	
}

void TestWorldLevel::load() {	
	//WorldBuilderDefinition def;
	//world = Main::getWorld()->builder->build(def);	

}

void TestWorldLevel::postLoad() {	
	//world->finalize(bgShader, treeShader);
	//Main::getAI()->create();
	//bgShader = reinterpret_cast<ShaderProgram*>(Main::getAssetManager()->getAsset("assets/shader/bg_shader")->data);
	//treeShader = reinterpret_cast<ShaderProgram*>(Main::getAssetManager()->getAsset("assets/shader/tree_shader")->data);
	
	modelShader = reinterpret_cast<ShaderProgram*>(M_Asset->getAsset("assets/shader/simple forward/sb_sf")->data);
	model = reinterpret_cast<Model*>(M_Asset->getAsset("assets/3d/deer/Model/Deer_old.dae")->data);
	mTex = reinterpret_cast<sf::Texture*>(M_Asset->getAsset("assets/3d/deer/Textures/Deer Common.png")->data);

	App::Gdx::printOpenGlErrors("test");
	view = M_View->create("main", ViewType::pers, true);
	view->setViewportBounds(0, 0, M_WIDTH, M_HEIGHT);
	auto cam = reinterpret_cast<ArcballCamera*>(view->getCamera());
	cam ->distance = 400.f;
	cam->fieldOfView = 67.f;
	cam->nearPlane = 0.1f;
	cam->farPlane = 1000.f;
	cam->arcball(cam->target, cam->azimuth, cam->height, cam->distance);
	
	view->setInteractive(true);
	view->panXModifier = 0.25f;
	view->panYModifier = 0.2f;
	view->zoomBounds = Vec2(10.f, 1000.f);
	view->zoomModifier = 25.f;
}

void TestWorldLevel::update() {
	view->clear(sf::Color::Black);
	//auto c = reinterpret_cast<ArcballCamera*>(view->getCamera());
	//c->arcball(c->target, c->azimuth += 0.5f, c->height, c->distance);
	view->apply();	
}

void TestWorldLevel::draw() {
	//for (auto v : world->bgVAOs)
		//v->draw(bgShader);
	//for (auto v : world->indexVAOs)
		//v->draw(treeShader);
	//Main::getAI()->draw(Main::getViewport()->cam.getTransform());

	modelShader->bind();
	uint uniforms = view->getUniformBuffer();
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, uniforms);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTex->getNativeHandle());

	App::Gdx::printOpenGlErrors("post draw");
	glBindVertexArray(model->vao);
	auto mesh = model->meshList[5];
	glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, (void*)(mesh->indexOffset * sizeof(uint)));
	App::Gdx::printOpenGlErrors("post draw");
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	modelShader->unbind();

	App::Gdx::printOpenGlErrors("post draw");
}

