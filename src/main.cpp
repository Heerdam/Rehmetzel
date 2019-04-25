
#include "MainStruct.hpp"

#include "InputMultiplexer.hpp"
#include "CameraUtils.hpp"
#include "World.hpp"
#include "Assets.hpp"
#include "UI.hpp"
#include "Level.h"
#include "Utils.hpp"

using namespace Heerbann;

App::Main* App::Main::instance = new App::Main();

int main() {
	

	MainConfig* config = new MainConfig();
	config->name = "Rehmetzel a0.3";
	config->windowWidth = 1980u;
	config->windowHeight = 1080u;

	M_Main->intialize(config);

	bool close = false;
	InputEntry* entry = new InputEntry();
	entry->closeEvent = [&]()->bool {
		close = true;
		return true;
	};
	entry->resizeEvent = [&](int _width, int _height)->bool {
		//TODO Main::getViewport()->setSize(_width, _height);
		return false;
	};
	M_Input->add("closeListener", entry);

	//Main::getViewport()->apply(*Main::getContext(), 1);
	M_Context->display();

	sf::Event event;
	while (M_Context->isOpen()) {
		if (close) {
			M_Context->close();
			delete M_Main;
			return 0;
		}
		try {
			auto start = std::chrono::system_clock::now();
			M_Main->update();
			while (M_Context->pollEvent(event))
				M_Input->fire(event);

			const float delta = 1.f / 60.f;

			M_World->update(delta);
			//update & apply
			Main::getViewport()->apply(*M_Context, delta);

			M_Level->update(delta);
			M_Level->draw(delta, M_Batch);

			M_Stage->act();
			M_Stage->draw(M_Batch);

			M_Context->build();
			M_Context->drawToScreen(Main::getViewport()->cam.getTransform());

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
		M_Context->display();
	}
	return 0;
}