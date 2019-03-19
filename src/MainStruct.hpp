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

	class InputMultiplexer;
	class World;
	class Viewport;
	class AssetManager;
	class LevelManager;

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

		//---------------------- Inputs ----------------------\\

		static InputMultiplexer* getInput();

		//---------------------- World ----------------------\\

		static World* getWorld();

		//---------------------- Viewport ----------------------\\

		static Viewport* getViewport();

		//---------------------- Assets ----------------------\\

		static AssetManager* getAssetManager();
		static sf::Font* getDefaultFont();

		//---------------------- Stage ----------------------\\

		static UI::Stage* getStage();

		//---------------------- Level ----------------------\\

		static LevelManager* getLevel();
		
	};	

}


