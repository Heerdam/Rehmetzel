
#include "MainStruct.hpp"

#include "InputMultiplexer.hpp"
#include "CameraUtils.hpp"
#include "World.hpp"

using namespace Heerbann;

MainStruct* MainStruct::instance = new MainStruct();

int main() {

	MainStruct::get()->canvasWidth = 1920;
	MainStruct::get()->canvasHeight = 1080;

	MainStruct::get()->inputListener = new InputMultiplexer();
	MainStruct::get()->world = new World();
	auto cam = MainStruct::get()->mainCam = new Viewport("main", -100);
	//cam debug
	cam->interactive = true;
	cam->debugDraw = true;
	cam->border = 15;
	cam->topBorder = 30;
	cam->setPosition(200, 200);
	cam->setSize(500, 500);

	sf::RenderWindow window(sf::VideoMode(MainStruct::get()->canvasWidth, MainStruct::get()->canvasHeight), "Judihui");
	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);

	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);

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

	sf::Event event;
	while (window.isOpen()) {
		while (window.pollEvent(event)) {
			MainStruct::get()->inputListener->fire(event);
		}

		window.setView(window.getDefaultView());
		window.clear(sf::Color::Black);

		MainStruct::get()->mainCam->apply(window, 1);
		//window.draw(shape);
		window.display();
	}

	return 0;
}