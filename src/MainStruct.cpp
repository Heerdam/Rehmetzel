
#include "MainStruct.hpp"

#include "InputMultiplexer.hpp"
#include "World.hpp"
#include "Assets.hpp"
#include "CameraUtils.hpp"
#include "UI.hpp"
#include "Level.h"

using namespace Heerbann;

Main::Main() {}

void Heerbann::Main::intialize() {

	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 4;
	settings.majorVersion = 4;
	settings.minorVersion = 1;

	context = new sf::RenderWindow();
	context->create(sf::VideoMode(64, 480, 32), "Rehmetzel a.0.1", sf::Style::Default, settings);
	context->setVerticalSyncEnabled(true);
	context->setFramerateLimit(60);	

	inputListener = new InputMultiplexer();
	world = new World();
	assets = new AssetManager();
	stage = new UI::Stage();
	level = new LevelManager();
	mainCam = new Viewport("main", -100);

	level->initialize();	
}

//---------------------- Context ----------------------\\

sf::RenderWindow* Main::getContext() {
	return instance->context;
}

void Main::setSize(unsigned int _width, unsigned int _height) {
	getContext()->setSize(sf::Vector2u(_width, _height));
}

//---------------------- Inputs ----------------------\\

InputMultiplexer* Main::getInput() {
	return instance->inputListener;
}

void Main::input_add(std::string _id, InputEntry* _entry) {
	getInput()->add(_id, _entry);
}

void Main::input_remove(InputEntry* _entry) {
	getInput()->remove(_entry);
}

//---------------------- World ----------------------\\

World* Main::getWorld() {
	return instance->world;
}

//---------------------- Viewport ----------------------\\

Viewport* Main::getViewport() {
	return instance->mainCam;
}

sf::View Heerbann::Main::viewport_getCamera() {
	return getViewport()->cam;
}

void Heerbann::Main::viewport_setSize(int _width, int _height) {
	getViewport()->setSize(_width, _height);
}

void Heerbann::Main::viewport_setPosition(int _x, int _y) {
	getViewport()->setPosition(_x, _y);
}

//---------------------- Assets ----------------------\\

AssetManager* Main::getAssetManager() {
	return instance->assets;
}

LoadItem* Main::asset_getAsset(std::string _id) {
	return getAssetManager()->getAsset(_id);
}

void Main::asset_addAsset(std::string _id, Type _type) {
	getAssetManager()->addAsset(_id, _type);
}

void Main::asset_load(std::string _id) {
	getAssetManager()->load(_id);
}

void Main::asset_unload(std::string _id) {
	getAssetManager()->unload(_id);
}

void Main::asset_startLoading() {
	getAssetManager()->startLoading();
}

bool Main::asset_isLoading() {
	return getAssetManager()->isLoading();
}

void Main::asset_finish() {
	getAssetManager()->finish();
}

void Main::asset_toggleState() {
	getAssetManager()->toggleState();
}

State Main::asset_getState() {
	return getAssetManager()->getState();
}

//---------------------- Stage ----------------------\\

UI::Stage* Main::getStage() {
	return get()->stage;
}

void Main::stage_add(UI::Actor* _actor) {
	getStage()->add(_actor);
}

//---------------------- Level ----------------------\\

LevelManager* Main::getLevel() {
	return get()->level;
}

void Heerbann::Main::loadLevel(std::string _id) {
	getLevel()->loadLevel(_id);
}

void Heerbann::Main::unloadLevel(std::string _id) {
	getLevel()->unloadLevel(_id);
}

