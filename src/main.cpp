
#include "MainStruct.hpp"
#include <iostream>

#include "InputMultiplexer.hpp"
#include "CameraUtils.hpp"
#include "World.hpp"
#include "Assets.hpp"
#include "UI.hpp"
#include "Level.h"
#include "Utils.hpp"

using namespace Heerbann;

Main* Main::instance = new Main();

int main() {

	Main::get()->intialize();

	Main::setSize(1920, 1080);
	Main::getViewport()->clearColor = sf::Color::Black;
	Main::getViewport()->setSize(1920, 1080);

	InputEntry* entry = new InputEntry();
	entry->closeEvent = [&]()->bool {
		Main::getContext()->close();
		return true;
	};
	entry->resizeEvent = [&](int _width, int _height)->bool {
		Main::getViewport()->setSize(_width, _height);
		return false;
	};
	Main::getInput()->add("closeListener", entry);

	Main::getViewport()->apply(*Main::getContext(), 1);
	Main::getContext()->display();

	Box2dRenderer boxRenderer;
	sf::Event event;
	while (Main::getContext()->isOpen()) {
		try {
			auto start = std::chrono::system_clock::now();
			Main::get()->update();
			while (Main::getContext()->pollEvent(event))
				Main::getInput()->fire(event);

			const float delta = 1.f / 60.f;

			Main::getWorld()->update(delta);
			//update & apply
			Main::getViewport()->apply(*Main::getContext(), delta);

			Main::getLevel()->update(delta);
			Main::getLevel()->draw(delta, Main::getBatch());

			//Main::getStage()->act(delta);
			//Main::getStage()->draw(Main::getBatch());

			Main::getBatch()->build();
			Main::getBatch()->drawToScreen(Main::getViewport()->cam.getTransform());


			//boxRenderer.draw(delta, *Main::getContext());
			//Main::getWorld()->debugDraw();

			

			auto end = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = end - start;
			//std::cout << elapsed_seconds.count() << std::endl;
		} catch (...) {
			std::exception_ptr p = std::current_exception();
			//std::cout << (p ? p._Current_exception->name : "null") << std::endl;
		}
		Main::getContext()->display();
	}

	return 0;
}