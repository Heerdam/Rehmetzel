
#include "MainStruct.hpp"

#include "InputMultiplexer.hpp"
#include "World.hpp"
#include "Assets.hpp"
#include "CameraUtils.hpp"
#include "UI.hpp"
#include "Level.h"
#include "Utils.hpp"
#include "TextUtil.hpp"
#include "AI.hpp"

using namespace Heerbann;

Main::Main() {}

Main::~Main() {
	delete batch;
	if(indexBuffer != nullptr) delete indexBuffer;
}

void Main::update() {
	++frameId;

	while (!loadJob.empty()) {
		std::tuple tuple = loadJob.front();
		loadJob.pop();
		std::get<0>(tuple)(std::get<1>(tuple));
	}
}

void Main::intialize() {

	sf::ContextSettings settings;
	settings.majorVersion = 4;
	settings.minorVersion = 6;

	context = new sf::RenderWindow();
	context->create(sf::VideoMode(640, 480, 32), "Rehmetzel a.0.2", sf::Style::Default, settings);
	//context->setVerticalSyncEnabled(true);
	context->setFramerateLimit(60);	

	inputListener = new InputMultiplexer();
	world = new World();
	assets = new AssetManager();
	stage = new UI::Stage();
	level = new LevelManager();
	
	mainCam = new Viewport("main", -100);

	indexBuffer = new GLuint[MAXSPRITES * 6];
	for (int i = 0; i < MAXSPRITES; ++i) {
		int k = 0;
		indexBuffer[i * 6] = 4 * i;
		indexBuffer[i * 6 + ++k] = 4 * i + 1;
		indexBuffer[i * 6 + ++k] = 4 * i + 2;

		indexBuffer[i * 6 + ++k] = 4 * i + 2;
		indexBuffer[i * 6 + ++k] = 4 * i + 3;
		indexBuffer[i * 6 + ++k] = 4 * i;
	}
	

	glewExperimental = GL_TRUE;
	auto status = glewInit();
	if (!status == GLEW_OK) {
		std::cout << "glew not ok" << std::endl;
	}

	//------------- Everything needing openGl goes below this line -------------\\

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	getAssetManager()->addAsset("assets/fonts/default.ttf", Type::font);
	getAssetManager()->load("assets/fonts/default.ttf");
	getAssetManager()->finish();
	update();

	defaultFont = reinterpret_cast<sf::Font*>(getAssetManager()->getAsset("assets/fonts/default.ttf")->data);

	intializeFont(getDefaultFont());

	cache = new Text::FontCache();
	cache->addFont(s2ws("default"), getDefaultFont());
	
	batch = new SpriteBatch(TEXTURECOUNT, MAXSPRITES);
	
	batch->addTexture(getDefaultFont());
	
	level->initialize();

	aiHandler = new AI::AIHandler();
	
}

std::wstring Main::s2ws(const char* _in) {
	return s2ws(std::string(_in));
}

 float Main::toFloatBits(int _r, int _g, int _b, int _a) {
	int color = (((int)(_a) << 24) | ((int)(_b) << 16) | ((int)(_g) << 8) | ((int)(_r)))&0xfeffffff;
	return *reinterpret_cast<float*>(&color);
};

std::wstring Main::s2ws(const std::string& _str) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &_str[0], (int)_str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &_str[0], (int)_str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

std::vector<std::wstring> Heerbann::Main::split(std::wstring _in, std::wstring _del) {
	return Text::SplitFunctor(std::wregex(_del), _in);
}

//---------------------- Job ----------------------\\

void Main::addJob(std::function<void(void*)> _job, void* _entry) {
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

SpriteBatch * Heerbann::Main::getBatch() {
	return get()->batch;
}

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

//---------------------- AI ----------------------\\

AI::AIHandler* Main::getAI() {
	return get()->aiHandler;
}

//---------------------- Assets ----------------------\\

AssetManager* Main::getAssetManager() {
	return instance->assets;
}

GLuint* Heerbann::Main::getIndexBuffer() {
	return get()->indexBuffer;
}


Text::FontCache* Main::getFontCache() {
	return get()->cache;
}

sf::Font* Main::getDefaultFont() {
	return get()->defaultFont;
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
