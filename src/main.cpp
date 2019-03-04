
#include "MainStruct.hpp"

using namespace Heerbann;

int main() {

	sf::RenderWindow window(sf::VideoMode(1920, 1080), "Judihui");
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
		shape.setPosition(sf::Vector2f((float)_x, (float)_y));
		return false;
	};

	MainStruct::get()->inputListener.add("closeListener", entry);

	sf::Event event;
	while (window.isOpen()) {
		while (window.pollEvent(event)) {
			MainStruct::get()->inputListener.fire(event);
		}
		window.clear();
		window.draw(shape);
		window.display();
	}

	return 0;
}