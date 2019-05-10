
#include "MainStruct.hpp"

#include "InputMultiplexer.hpp"
#include "CameraUtils.hpp"
#include "World.hpp"
#include "Assets.hpp"
#include "UI.hpp"
#include "Level.h"
#include "Utils.hpp"
#include "TimeLog.hpp"
#include "Gdx.hpp"

using namespace Heerbann;

App::Main* App::Main::instance = new App::Main();

int main() {
	
	MainConfig* config = new MainConfig();
	config->name = "Rehmetzel a0.3";
	config->windowWidth = 1980u;
	config->windowHeight = 1080u;
	config->settings.majorVersion = 4;
	config->settings.minorVersion = 6;
	config->settings.stencilBits = 8;
	config->settings.depthBits = 24;

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
		auto start = TIMESTAMP;
		if (close) {
			M_Context->close();
			delete M_Main;
			return 0;
		}	
		try {
			
			M_Main->update();
			while (M_Context->pollEvent(event))
				M_Input->fire(event);

			const float delta = 1.f / 60.f;

			//update & apply
			M_Env->update();

			M_Level->update();
			M_Level->draw();

			//M_Stage->act();
			//M_Stage->draw(M_Batch);
	
		} catch (const std::runtime_error& re) {
			std::cerr << "Runtime error: " << re.what() << std::endl;
		} catch (const std::exception& ex) {
			std::cerr << "Error occurred: " << ex.what() << std::endl;
		} catch (...) {
			std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
		}
		unsigned long long elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(TIMESTAMP.time - start.time).count();
		const uint NsToMs = 1000000;
		unsigned long long wait = (14 - elapsedTime) * NsToMs;
		auto sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, wait);
		M_Context->display();
	}
	return 0;
}