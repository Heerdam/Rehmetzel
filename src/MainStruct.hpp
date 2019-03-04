#ifndef MAINSTRUCT_H
#define MAINSTRUCT_H

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "InputMultiplexer.hpp"
#include "World.hpp"

namespace Heerbann {

	class MainStruct {
	public:
		InputMultiplexer inputListener;
		World world;

	private:
		static MainStruct* instance;
		MainStruct() {
		};

	public:
		static MainStruct* get() {
			return instance;
		};
		
	};

	MainStruct* MainStruct::instance = new MainStruct();

}

#endif
