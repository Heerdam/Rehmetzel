
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
	bool close = false;

	Main::get()->intialize();

	Main::setSize(1920, 1080);
	Main::getViewport()->clearColor = sf::Color::Black;
	Main::getViewport()->setSize(1920, 1080);

	InputEntry* entry = new InputEntry();
	entry->closeEvent = [&]()->bool {
		close = true;
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
		if (close) {
			Main::getContext()->close();
			delete Main::get();
			return 0;
		}
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

			Main::getStage()->act();
			Main::getStage()->draw(Main::getBatch());

			Main::getBatch()->build();
			Main::getBatch()->drawToScreen(Main::getViewport()->cam.getTransform());

			//boxRenderer.draw(delta, *Main::getContext());
			//Main::getWorld()->debugDraw();

			auto end = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = end - start;
			//std::cout << elapsed_seconds.count() << std::endl;
		} catch (const std::runtime_error& re) {
			std::cerr << "Runtime error: " << re.what() << std::endl;
		} catch (const std::exception& ex) {
			std::cerr << "Error occurred: " << ex.what() << std::endl;
		} catch (...) {
			std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
		}
		Main::getContext()->display();
	}
	return 0;
}