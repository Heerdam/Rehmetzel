#ifndef MAINSTRUCT_H
#define MAINSTRUCT_H

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

	class MainStruct {
	public:
		InputMultiplexer* inputListener;
		World* world;
		Viewport* mainCam;
		AssetManager* assets;

		int canvasWidth = 640;
		int canvasHeight = 480;

	private:
		static MainStruct* instance;
		MainStruct();

	public:
		static MainStruct* get() {
			return instance;
		};

		long static getId() {
			static long id = 1000;
			return ++id;
		}
		
	};	

}

#endif

