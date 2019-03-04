#ifndef INPUTMULTIPLEXER_H
#define INPUTMULTIPLEXER_H

#include <vector>
#include <unordered_map>
#include <functional>

#include "MainStruct.hpp"

namespace Heerbann {

	using namespace Heerbann;

	class MainStruct;

	class InputMultiplexer {

	public:
		struct InputEntry {

			//interrupts if event returns true
			bool interrupting = true; 

			//the smaller the priority, the earlier it will get called
			int priority = 0;

			// Code of the key that has been pressed
			// Is the Alt key pressed?
			// Is the Control key pressed?
			// Is the Shift key pressed?
			// Is the System key pressed?
			std::function<bool(sf::Keyboard::Key, bool, bool, bool, bool)> keyPressEvent = nullptr;

			// Code of the key that has been released
			// Is the Alt key pressed?
			// Is the Control key pressed?
			// Is the Shift key pressed?
			// Is the System key pressed?
			std::function<bool(sf::Keyboard::Key, bool, bool, bool, bool)> keyReleaseEvent = nullptr;

			// UTF-32 Unicode value of the character
			std::function<bool(sf::Uint32)> textEvent = nullptr;

			// X position of the mouse pointer, relative to the left of the owner window
			// Y position of the mouse pointer, relative to the top of the owner window
			std::function<bool(int, int)> mouseMoveEvent = nullptr;

			// The mouse cursor entered the area of the window (no data)
			std::function<bool()> mouseEnterEvent = nullptr;

			// The mouse cursor left the area of the window (no data)
			std::function<bool()> mouseLeftEvent = nullptr;

			// Code of the button that has been pressed
			// X position of the mouse pointer, relative to the left of the owner window
			// Y position of the mouse pointer, relative to the top of the owner window
			std::function<bool(sf::Mouse::Button, int, int)> mouseButtonPressEvent = nullptr;

			// Code of the button that has been released
			// X position of the mouse pointer, relative to the left of the owner window
			// Y position of the mouse pointer, relative to the top of the owner window
			std::function<bool(sf::Mouse::Button, int, int)> mouseButtonReleaseEvent = nullptr;

			// Which wheel (for mice with multiple ones)
			// Wheel offset (positive is up/left, negative is down/right). High-precision mice may use non-integral offsets.
			// X position of the mouse pointer, relative to the left of the owner window
			// Y position of the mouse pointer, relative to the top of the owner window
			std::function<bool(sf::Mouse::Wheel, float, int, int)> mouseWheelScrollEvent = nullptr;

			// Index of the joystick (in range [0 .. Joystick::Count - 1])
			std::function<bool(unsigned int)> joystickConnectEvent = nullptr;

			// Index of the joystick (in range [0 .. Joystick::Count - 1])
			std::function<bool(unsigned int)> joystickDisconnectEvent = nullptr;

			// Index of the joystick (in range [0 .. Joystick::Count - 1])
			// Axis on which the joystick moved
			// New position on the axis (in range [-100 .. 100])
			std::function<bool(unsigned int, sf::Joystick::Axis, float)> joystickMoveEvent = nullptr;

			// Index of the joystick (in range [0 .. Joystick::Count - 1])
			// Index of the button that has been pressed (in range [0 .. Joystick::ButtonCount - 1])
			std::function<bool(unsigned int, unsigned int)> joystickButtonPressEvent = nullptr;

			// Index of the joystick (in range [0 .. Joystick::Count - 1])
			// Index of the button that has been released (in range [0 .. Joystick::ButtonCount - 1])
			std::function<bool(unsigned int, unsigned int)> joystickButtonReleaseEvent = nullptr;

			// Index of the finger in case of multi-touch events
			// X position of the touch, relative to the left of the owner window
			// Y position of the touch, relative to the top of the owner window
			std::function<bool(unsigned int, int, int)> touchBeginEvent = nullptr;

			// Index of the finger in case of multi-touch events
			// X position of the touch, relative to the left of the owner window
			// Y position of the touch, relative to the top of the owner window
			std::function<bool(unsigned int, int, int)> touchEndEvent = nullptr;

			// Index of the finger in case of multi-touch events
			// X position of the touch, relative to the left of the owner window
			// Y position of the touch, relative to the top of the owner window
			std::function<bool(unsigned int, int, int)> touchMoveEvent = nullptr;

			// Type of the sensor
			// Current value of the sensor on X axis
			// Current value of the sensor on Y axis
			// Current value of the sensor on Z axis
			std::function<bool(sf::Sensor::Type, float, float, float)> sensorEvent = nullptr;

			// ---------APPLICATION LISTENER--------- 

			// The window requested to be closed (no data)
			std::function<bool()> closeEvent = nullptr;

			// New width, in pixels
			// New height, in pixels
			std::function<bool(unsigned int, unsigned int)> resizeEvent = nullptr;

			// The window lost the focus (no data)
			std::function<bool()> lostFocusEvent = nullptr;

			// The window gained the focus (no data)
			std::function<bool()> gainedFocusEvent = nullptr;
		};

	private:

		std::unordered_map<std::string, InputEntry*> entries_cache;

		//0 - keyPressEvent
		//1 - keyReleaseEvent
		//2 - textEvent
		//3 - mouseMoveEvent
		//4 - mouseEnterEvent
		//5 - mouseLeftEvent
		//6 - mouseButtonPressEvent
		//7 - mouseButtonReleaseEvent
		//8 - mouseWheelScrollEvent
		//9 - joystickConnectEvent
		//10 - joystickDisconnectEvent
		//11 - joystickMoveEvent
		//12 - joystickButtonPressEvent
		//13 - joystickButtonReleaseEvent
		//14 - touchBeginEvent
		//15 - touchEndEvent
		//16 - touchMoveEvent
		//17 - closeEvent
		//18 - resizeEvent
		//19 - lostFocusEvent
		//20 - gainedFocusEvent
		std::vector<std::vector<InputEntry*>> entries = std::vector<std::vector<InputEntry*>>(21);

		std::function<bool(InputEntry*, InputEntry*)> sort = [&](InputEntry* _i1, InputEntry* _i2)->bool {
			return _i1->priority <= _i2->priority;
		};

	private:
		void update();

	public:
		void add(std::string _id, InputEntry* _entry);

		InputEntry* operator[](std::string _id) {
			if (entries_cache.count(_id) == 0) return nullptr;
			return entries_cache[_id];
		};
	
		void remove(InputEntry* _entry);

		bool fire(sf::Event& _event);
	
	};

}

#endif