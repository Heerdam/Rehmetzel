#ifndef MAINSTRUCT_H
#define MAINSTRUCT_H

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/OpenGL.hpp>

//#include "InputMultiplexer.hpp"
//#include "World.hpp"
//#include "CameraUtils.hpp"

namespace Heerbann {

	using namespace Heerbann;

	class InputMultiplexer;
	class World;
	class Viewport;

	class MainStruct {
	public:
		InputMultiplexer* inputListener;
		World* world;
		Viewport* mainCam;

		int canvasWidth;
		int canvasHeight;

	private:
		static MainStruct* instance;
		MainStruct() {
		};

	public:
		static MainStruct* get() {
			return instance;
		};
		
	};

}

#endif

