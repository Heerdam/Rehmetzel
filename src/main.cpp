
#include "MainStruct.hpp"
#include <iostream>

#include "InputMultiplexer.hpp"
#include "CameraUtils.hpp"
#include "World.hpp"
#include "Assets.hpp"
#include "UI.hpp"
#include "Level.h"

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
		Main::viewport_setSize(_width, _height);
		return false;
	};
	Main::input_add("closeListener", entry);

	Main::getViewport()->apply(*Main::getContext(), 1);
	Main::getContext()->display();

	Box2dRenderer boxRenderer;
	


	sf::Event event;
	while (Main::getContext()->isOpen()) {
		Main::get()->update();
		while (Main::getContext()->pollEvent(event))
			Main::getInput()->fire(event);

		const float delta = 1.f / 60.f;

		Main::getWorld()->update(delta);
		//update & apply
		Main::getViewport()->apply(*Main::getContext(), delta);

		Main::getLevel()->update(delta);
		Main::getLevel()->draw(delta, *Main::getContext());

		boxRenderer.draw(delta, *Main::getContext());

		Main::getWorld()->debugDraw();

		//Main::getStage()->act(delta);
		//Main::getStage()->draw(*Main::getContext());

		Main::getContext()->display();
	}

	return 0;
}