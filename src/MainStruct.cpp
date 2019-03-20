
#include "MainStruct.hpp"

#include "InputMultiplexer.hpp"
#include "World.hpp"
#include "Assets.hpp"
#include "CameraUtils.hpp"
#include "UI.hpp"
#include "Level.h"

using namespace Heerbann;

Main::Main() {}

void Heerbann::Main::update() {
	++frameId;

	while (!loadJob.empty()) {
		std::tuple tuple = loadJob.front();
		loadJob.pop();
		std::get<0>(tuple)(std::get<1>(tuple));
	}
}

void Heerbann::Main::intialize() {

	sf::ContextSettings settings;
	settings.majorVersion = 3;
	settings.minorVersion = 3;

	context = new sf::RenderWindow();
	context->create(sf::VideoMode(640, 480, 32), "Rehmetzel a.0.1", sf::Style::Default, settings);
	//context->setVerticalSyncEnabled(true);
	context->setFramerateLimit(60);	

	inputListener = new InputMultiplexer();
	world = new World();
	assets = new AssetManager();
	stage = new UI::Stage();
	level = new LevelManager();
	mainCam = new Viewport("main", -100);

	glewExperimental = GL_TRUE;
	auto status = glewInit();
	if (!status == GLEW_OK) {
		std::exception("glew not ok");
	}

	getAssetManager()->addAsset("assets/fonts/default.ttf", Type::font);
	getAssetManager()->load("assets/fonts/default.ttf");
	getAssetManager()->finish();
	update();

	intializeFont(getDefaultFont());
	
	level->initialize();
}

//---------------------- Job ----------------------\\

void Heerbann::Main::addJob(std::function<void(void*)> _job, void* _entry) {
	std::lock_guard<std::mutex> guard(instance->jobLock);
	instance->loadJob.emplace(std::make_tuple(_job, _entry));
}

//---------------------- Random ----------------------\\

void Main::setSeed(long _seed) {
	instance->random.seed(_seed);
}

float Main::getRandom() {
	return (float)((instance->random() >> 40) * 1.0 / (1L << 24));
}

float Main::getRandom(float _low, float _high) {
	return _low + getRandom() * (_high - _low);
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

//---------------------- World ----------------------\\

World* Main::getWorld() {
	return instance->world;
}

//---------------------- Viewport ----------------------\\

Viewport* Main::getViewport() {
	return instance->mainCam;
}

//---------------------- Assets ----------------------\\

AssetManager* Main::getAssetManager() {
	return instance->assets;
}

sf::Font* Main::getDefaultFont() {
	return (sf::Font*)getAssetManager()->getAsset("assets/fonts/default.ttf")->data;
}

void Heerbann::Main::intializeFont(sf::Font* _font) {
	//basic latins
	for (uint32 i = 32; i <= 126; ++i) {
		_font->getGlyph(i, SMALLFONTSIZE, false);
		_font->getGlyph(i, MEDIUMFONTSIZE, false);
		_font->getGlyph(i, BIGFONTSIZE, false);

		_font->getGlyph(i, SMALLFONTSIZE, true);
		_font->getGlyph(i, MEDIUMFONTSIZE, true);
		_font->getGlyph(i, BIGFONTSIZE, true);
	}
	//Latin-1 Supplement
	for (uint32 i = 160; i <= 255; ++i) {
		_font->getGlyph(i, SMALLFONTSIZE, false);
		_font->getGlyph(i, MEDIUMFONTSIZE, false);
		_font->getGlyph(i, BIGFONTSIZE, false);

		_font->getGlyph(i, SMALLFONTSIZE, true);
		_font->getGlyph(i, MEDIUMFONTSIZE, true);
		_font->getGlyph(i, BIGFONTSIZE, true);
	}
}



//---------------------- Stage ----------------------\\

UI::Stage* Main::getStage() {
	return get()->stage;
}

//---------------------- Level ----------------------\\

LevelManager* Main::getLevel() {
	return get()->level;
}
