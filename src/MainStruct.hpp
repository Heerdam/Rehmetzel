#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/OpenGL.hpp>
#include <unordered_map>

namespace Heerbann {

	using namespace Heerbann;

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

		static Main* instance;
		Main();

	public:

		void intialize();

		static Main* get() {
			return instance;
		};

		long static getId() {
			static long id = 1000;
			return ++id;
		}

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
		static void input_add(std::string, InputEntry*);
		static void input_remove(InputEntry*);

		//---------------------- World ----------------------\\

		static World* getWorld();

		//---------------------- Viewport ----------------------\\

		static Viewport* getViewport();
		static sf::View viewport_getCamera();
		static void viewport_setSize(int, int);
		static void viewport_setPosition(int, int);

		//---------------------- Assets ----------------------\\

		static AssetManager* getAssetManager();
		static LoadItem* asset_getAsset(std::string);
		static void asset_addAsset(std::string, Type);
		static void asset_load(std::string);
		static void asset_unload(std::string);
		static void asset_startLoading();
		static bool asset_isLoading();
		static void asset_finish();
		static void asset_toggleState();
		static State asset_getState();

		//---------------------- Stage ----------------------\\

		static UI::Stage* getStage();
		static void stage_add(UI::Actor*);

		//---------------------- Level ----------------------\\

		static LevelManager* getLevel();
		static void loadLevel(std::string);
		static void unloadLevel(std::string);
		
	};	

}


