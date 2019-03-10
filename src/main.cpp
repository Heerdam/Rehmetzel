
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
	bg.setTextureRect(sf::IntRect(0, 0, scale * 2048, scale * 2048));
	bg.setPosition(sf::Vector2f(0, 0));
	bg.setScale(sf::Vector2f(0.25f, 0.25f));
	bg.setSize(sf::Vector2f((float)scale * 2048.f, (float)scale * 2048.f));

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
		
		auto bounds = bg.getGlobalBounds();
		float imW = bounds.width;
		float imH = bounds.height;
		
		auto pos = cam->cam.getCenter();

		float width = cam->width*cam->zoom;
		float height = cam->height*cam->zoom;

		sf::Vector2f lCorner(pos.x - width/2, pos.y - height/2);
		
		float offsetX = lCorner.x - (int)(lCorner.x / width) * width;
		float offsetY = lCorner.y - (int)(lCorner.y / height) * height;

		int countX = (int)(width / imW) + 1;
		int countY = (int)(height / imH) + 1;

		for (int x = 0; x < countX; ++x) {
			for (int y = 0; y < countY; ++y) {
				bg.setPosition(sf::Vector2f(offsetX + x * imW, offsetY + y * imH));
				_window.draw(bg);
			}
		}


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