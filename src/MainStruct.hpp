#pragma once

#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <unordered_map>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>
#include <atomic>
#include <functional>
#include <algorithm>
#include <iostream>
#include <math.h>
#include <limits>
#include <random>
#include <sstream>
#include <tuple>
#include <chrono>
#include <stdexcept>
#include <exception>
#include <typeinfo>
#include <regex>
#include <windows.h>

#include <Box2D/Box2D.h>

namespace Heerbann {

	using namespace Heerbann;

// 1 meter (box2d) is more or less 64 pixels (sfml)
#define RATIO 30.0f
#define PIXELS_PER_METER RATIO

// 64 pixels (sfml) are more or less 1 meter (box2d)
#define UNRATIO (1.0F/RATIO)
#define METERS_PER_PIXEL UNRATIO

#define DEGTORAD (b2_pi / 180.0)

//diameter of a cell of the background
#define BG_CELLDIAMETER 100

//how many cells a background VAO holds
#define BG_CELLCOUNT 50

#define SMALLFONTSIZE 18
#define MEDIUMFONTSIZE 24
#define BIGFONTSIZE 32

//how many sprites the spritebatch holds
#define TEXTURECOUNT 10

	class InputMultiplexer;
	class World;
	class Viewport;
	class AssetManager;
	class LevelManager;
	class SpriteBatch;

	namespace Text {
		class FontCache;
	}

	struct InputEntry;
	struct LoadItem;

	enum State : int;
	enum Type : int;

	namespace UI {
		class Stage;
		class Actor;
	}

	class Main {
	private:
		sf::RenderWindow* context;
		InputMultiplexer* inputListener;
		World* world;
		Viewport* mainCam;
		AssetManager* assets;
		UI::Stage* stage;
		LevelManager* level;
		SpriteBatch* batch;
		Text::FontCache* cache;

		unsigned long frameId = 1;

		static Main* instance;
		Main();

		std::mt19937_64 random;

		std::queue<std::tuple<std::function<void(void*)>, void*>> loadJob;
		std::mutex jobLock;

	public:

		void update();
		void intialize();

		inline static unsigned long getFrameId() {
			return instance->frameId;
		};

		static Main* get() {
			return instance;
		};

		long static getId() {
			static long id = 1000;
			return ++id;
		}

		static std::wstring s2ws(const char*);
		static std::wstring s2ws(const std::string&);

		static std::vector<std::wstring> split(std::wstring, std::wstring);

		constexpr static float toFloatBits(int _r, int _g, int _b, int _a) {
			int color = (((int)(255 * _a) << 24) | ((int)(255 * _b) << 16) | ((int)(255 * _g) << 8) | ((int)(255 * _r))) & 0xfeffffff;
			return *reinterpret_cast<float*>(&color);
		};

		static float toFloatBits(sf::Color _color) {
			return toFloatBits(_color.r, _color.g, _color.b, _color.a);
		};

		bool almost_equal(float _f1, float _f2) {
			return false; //TODO
		}

		//---------------------- Font ----------------------\\

		static Text::FontCache* getFontCache();
		static sf::Font* getDefaultFont();
		static void intializeFont(sf::Font*);

		//---------------------- Job ----------------------\\

		//thread safe
		static void addJob(std::function<void(void*)>, void*);

		//---------------------- Random ----------------------\\

		static void setSeed(long);
		//random in interval [0, 1]
		static float getRandom();
		static float getRandom(float, float);

		//---------------------- Context ----------------------\\

		static sf::RenderWindow* getContext();
		static void setSize(unsigned int _width, unsigned int _height);

		inline static unsigned int width() {
			return get()->getContext()->getSize().x;
		};

		inline static unsigned int height() {
			return get()->getContext()->getSize().y;
		};

		//---------------------- Batch ----------------------\\

		static SpriteBatch* getBatch();

		//---------------------- Inputs ----------------------\\

		static InputMultiplexer* getInput();

		//---------------------- World ----------------------\\

		static World* getWorld();

		//---------------------- Viewport ----------------------\\

		static Viewport* getViewport();

		//---------------------- Assets ----------------------\\

		static AssetManager* getAssetManager();

		//---------------------- Stage ----------------------\\

		static UI::Stage* getStage();

		//---------------------- Level ----------------------\\

		static LevelManager* getLevel();
		
	};	

}


