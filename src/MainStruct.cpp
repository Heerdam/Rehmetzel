
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
#include "TimeLog.hpp"
#include "Gdx.hpp"
#include "World.hpp"

using namespace Heerbann;
using namespace App;

Main::Main() {}

Main::~Main() {
	delete batch;
	if(indexBuffer != nullptr) delete indexBuffer;
}

void Main::update() {
	++frameId;

	// if true is returned job is removed
	while (!loadJob.empty()) {
		std::tuple tuple = loadJob.front();		
		if(std::get<0>(tuple)(std::get<1>(tuple)));
			loadJob.pop();
	}
}

void Main::intialize(MainConfig* _config) {

	context = new sf::RenderWindow();
	context->create(sf::VideoMode(_config->windowWidth, _config->windowHeight, 32), _config->name, _config->windowStyle, _config->settings);
	//context->setVerticalSyncEnabled(true);
	context->setFramerateLimit(60);	

	inputListener = new InputMultiplexer();
	world = new VoxelWorld();
	assets = new AssetManager();
	stage = new UI::Stage();
	level = new LevelManager();
	timer = new Timer();
	logger = new Logger();
	env = new Environment();
	
	viewport = new ViewportHandler();

	indexBuffer = new GLuint[_config->MAXSPRITES * 6];
	for (uint i = 0; i < _config->MAXSPRITES; ++i) {
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

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	//glEnable(GL_SCISSOR_TEST);
	
	new ShaderProgram("assets/shader/spritebatch/sb_sprite");
	new Font("assets/fonts/default.ttf");

	//getAssetManager()->addAsset("assets/fonts/default.ttf", Type::font);
	//getAssetManager()->load("assets/fonts/default.ttf");
	//getAssetManager()->addAsset("assets/shader/spritebatch/sb_sprite", Type::shader);
	//getAssetManager()->load("assets/shader/spritebatch/sb_sprite");
	getAssetManager()->finish();
	update();

	defaultFont = reinterpret_cast<sf::Font*>(getAssetManager()->getAsset("assets/fonts/default.ttf")->data);

	intializeFont(getDefaultFont());

	cache = new Text::FontCache();
	cache->addFont(Util::s2ws("default"), getDefaultFont());
	
	//batch = new SpriteBatch(_config->MAXSPRITES);
	
	level->initialize();
	env->initialize();

	//aiHandler = new AI::AIHandler();
	
	delete _config;
}

Main* App::Main::getInstance() {
	return instance;
}

std::wstring Util::s2ws(const char* _in) {
	return s2ws(std::string(_in));
}

 float Util::toFloatBits(int _r, int _g, int _b, int _a) {
	int color = (((int)(_a) << 24) | ((int)(_b) << 16) | ((int)(_g) << 8) | ((int)(_r)))&0xfeffffff;
	return *reinterpret_cast<float*>(&color);
}

std::wstring Util::s2ws(const std::string& _str) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &_str[0], (int)_str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &_str[0], (int)_str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

std::vector<std::wstring> Util::split(std::wstring _in, std::wstring _del) {
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

ShapeRenderer* App::Main::getShape() {
	return getInstance()->shape;
}

Timer* App::Main::getTimer() {
	return getInstance()->timer;
}

Logger* App::Main::getLogger() {
	return getInstance()->logger;
}

SpriteBatch* Main::getBatch() {
	return getInstance()->batch;
}

InputMultiplexer* Main::getInput() {
	return instance->inputListener;
}

//---------------------- World ----------------------\\

VoxelWorld* Main::getWorld() {
	return instance->world;
}

Environment* App::Main::getEnv() {
	return getInstance()->env;
}

//---------------------- Viewport ----------------------\\

ViewportHandler* Main::getViewport() {
	return instance->viewport;
}

//---------------------- AI ----------------------\\

//---------------------- Assets ----------------------\\

AssetManager* Main::getAssetManager() {
	return instance->assets;
}

GLuint* Main::getIndexBuffer() {
	return getInstance()->indexBuffer;
}

float App::Main::deltaTime() {
	return 0.0f;
}


Text::FontCache* Main::getFontCache() {
	return getInstance()->cache;
}

sf::Font* Main::getDefaultFont() {
	return getInstance()->defaultFont;
}

void Main::intializeFont(sf::Font* _font) {
	/*
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
	*/
}

//---------------------- Stage ----------------------\\

UI::Stage* Main::getStage() {
	return getInstance()->stage;
}

//---------------------- Level ----------------------\\

LevelManager* Main::getLevel() {
	return getInstance()->level;
}

