
#include "Level.h"
#include "Assets.hpp"
#include "UI.hpp"
#include "CameraUtils.hpp"
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

void Heerbann::LevelManager::queueLevelToLoad(std::string _id) {
	auto assets = Main::getAssetManager();
	Level* level = assets->getLevel(_id);
	toLoad.emplace(level);
}

void Heerbann::LevelManager::queueLevelToUnLoad(std::string _id) {
	auto assets = Main::getAssetManager();
	Level* level = assets->getLevel(_id);
	toUnload.emplace(level);
}

void Heerbann::LevelManager::queueLevelToLoad(Level* _level) {
	toLoad.emplace(_level);
}

void Heerbann::LevelManager::queueLevelToUnLoad(Level* _level) {
	toUnload.emplace(_level);
}

//---------------------- PreLoadLevel ----------------------\\

void PreLoadLevel::preLoad(AssetManager* _asset) {
	//assetToLoad.emplace_back(new LoadItem("assets/fonts/black.ttf", Type::font));
}

void PreLoadLevel::postLoad(AssetManager* _assets) {
	Main::getLevel()->queueLevelToUnLoad(this);
	Main::getLevel()->queueLevelToLoad("LoadingScreenLevel");
}

//---------------------- LoadingScreenLevel ----------------------\\

void LoadingScreenLevel::preLoad(AssetManager *) {
	assetToLoad.emplace_back(new LoadItem("assets/tex/Forest_soil_diffuse.png", Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestCliff_basecolor.png", Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestDirt_diffuse.png", Type::texture));

	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestGrass_basecolor.png", Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestMoss_basecolor.png", Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestMud_baseColor.png", Type::texture));

	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestRoad_diffuse.png", Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestRock_basecolor.png", Type::texture));
	assetToLoad.emplace_back(new LoadItem("assets/tex/ForestWetMud_baseColor.png", Type::texture));
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

void Heerbann::TestWorldLevel::preLoad(AssetManager *) {
	
	
}

void TestWorldLevel::load(AssetManager* _asset) {
	
	pos = new float[vertexCount * 3]{
	 0.f, 0.f, 0.f,
	 200.f, 0.f, 1.f,
	 400.f, 0.f, 2.f,

	 0.f, 225.f, 3.f,
	 200.f, 225.f, 4.f,
	 400.f, 225.f, 5.f,

	 0.f, 450.f, 6.f,
	 200.f, 450.f, 7.f,
	 400.f, 450.f, 8.f,
	};
}

void TestWorldLevel::postLoad(AssetManager* _asset) {
	
	bgShader = new sf::Shader();
	if (!bgShader->loadFromFile("assets/shader/bg_shader.vert", "assets/shader/bg_shader.geom", "assets/shader/bg_shader.frag"))
		std::exception("vertex/ geom/ fragment failed");

	//GLint link;
	//glGetProgramiv(bgShader->getNativeHandle(), GL_LINK_STATUS, &link);
	//std::cout << "link: " << link << std::endl;

	cameraUniformHandle = glGetUniformLocation(bgShader->getNativeHandle(), "transform");

	//create buffer
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 3, pos, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	for (int i = 0; i < 9; ++i)
		tex[i] = new sf::Texture();

	tex[0]->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/tex/Forest_soil_diffuse.png")->data));

	tex[1]->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/tex/ForestCliff_basecolor.png")->data));
	tex[2]->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/tex/ForestDirt_diffuse.png")->data));

	tex[3]->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/tex/ForestGrass_basecolor.png")->data));
	tex[4]->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/tex/ForestMoss_basecolor.png")->data));
	tex[5]->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/tex/ForestMud_baseColor.png")->data));

	tex[6]->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/tex/ForestRoad_diffuse.png")->data));
	tex[7]->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/tex/ForestRock_basecolor.png")->data));
	tex[8]->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/tex/ForestWetMud_baseColor.png")->data));

	for (int i = 0; i < 9; ++i)
		texLoc[i] = glGetUniformLocation(bgShader->getNativeHandle(), (std::string("tex[") + std::to_string(i) + std::string("]")).c_str());

	//GLenum err;
	//while ((err = glGetError()) != GL_NO_ERROR) {
	//	std::cout << err << std::endl;
	//}
}

void TestWorldLevel::update(float _delta) {
}

void TestWorldLevel::draw(float _delta, sf::RenderWindow& _window) {
	sf::Shader::bind(bgShader);
	glUniformMatrix4fv(cameraUniformHandle, 1, false, Main::getViewport()->cam.getTransform().getMatrix());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex[0]->getNativeHandle());
	glUniform1i(texLoc[0], 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex[1]->getNativeHandle());
	glUniform1i(texLoc[1], 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex[2]->getNativeHandle());
	glUniform1i(texLoc[2], 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, tex[3]->getNativeHandle());
	glUniform1i(texLoc[3], 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, tex[4]->getNativeHandle());
	glUniform1i(texLoc[4], 4);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, tex[5]->getNativeHandle());
	glUniform1i(texLoc[5], 5);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, tex[6]->getNativeHandle());
	glUniform1i(texLoc[6], 6);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, tex[7]->getNativeHandle());
	glUniform1i(texLoc[7], 7);

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, tex[8]->getNativeHandle());
	glUniform1i(texLoc[8], 8);

	//GLenum err;
	//while ((err = glGetError()) != GL_NO_ERROR) {
	//	std::cout << err << std::endl;
	//}

	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, vertexCount); //TODO

	glBindVertexArray(0);

	glUseProgram(0);
}

