
#include "MainStruct.hpp"
#include <iostream>

#include "InputMultiplexer.hpp"
#include "CameraUtils.hpp"
#include "World.hpp"
#include "Assets.hpp"

using namespace Heerbann;

MainStruct* MainStruct::instance = new MainStruct();

sf::Texture* loadAssets() {
	MainStruct::get()->assets->load("assets/tex/ForestGrass_basecolor.png", Type::texture);
	MainStruct::get()->assets->finish();
	sf::Texture* texture = (sf::Texture*)(*MainStruct::get()->assets)["assets/tex/ForestGrass_basecolor.png"]->data;	
	texture->setRepeated(true);
	return texture;
}

int main() {

	MainStruct::get()->canvasWidth = 1920;
	MainStruct::get()->canvasHeight = 1080;

	auto cam = MainStruct::get()->mainCam = new Viewport("main", -100);
	cam->clearColor = sf::Color::Black;

	sf::RenderWindow window(sf::VideoMode(MainStruct::get()->canvasWidth, MainStruct::get()->canvasHeight), "Rehmetzel a.0.1");
	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);

	int scale = 6;
	sf::RectangleShape bg;
	bg.setTexture(loadAssets());
	bg.setTextureRect(sf::IntRect(0, 0, scale * 2048.f, scale * 2048.f));
	bg.setPosition(sf::Vector2f(0, 0));
	bg.setScale(sf::Vector2f(0.25f, 0.25f));
	bg.setSize(sf::Vector2f(scale * 2048.f, scale * 2048.f));

	InputMultiplexer::InputEntry* entry = new InputMultiplexer::InputEntry();
	entry->closeEvent = [&]()->bool {
		window.close();
		return true;
	};
	entry->mouseMoveEvent = [&](int _x, int _y)->bool{
		//shape.setPosition(sf::Vector2f((float)_x, (float)_y));
		return false;
	};
	entry->resizeEvent = [&](int _width, int _height)->bool{
		MainStruct::get()->canvasWidth = _width;
		MainStruct::get()->canvasHeight = _height;
		return false;
	};

	MainStruct::get()->inputListener->add("closeListener", entry);

	cam->update = [&](sf::RenderWindow& _window, float _deltaTime)->void {
		_window.draw(bg);
	};

	sf::Event event;
	while (window.isOpen()) {
		while (window.pollEvent(event)) {
			MainStruct::get()->inputListener->fire(event);
		}

		MainStruct::get()->mainCam->apply(window, 1);
		window.display();
	}

	return 0;
}