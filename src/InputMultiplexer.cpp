
#include "InputMultiplexer.hpp"

using namespace Heerbann;

void InputMultiplexer::update() {
	for (auto& o : entries)
		std::sort(o.begin(), o.end(), sort);
}

void InputMultiplexer::add(std::string _id, InputEntry* _entry) {
	entries_cache[_id] = _entry;
	if (_entry->keyPressEvent != nullptr)
		entries[0].emplace_back(_entry);

	if (_entry->keyReleaseEvent != nullptr)
		entries[1].emplace_back(_entry);
	if (_entry->textEvent != nullptr)
		entries[2].emplace_back(_entry);
	if (_entry->mouseMoveEvent != nullptr)
		entries[3].emplace_back(_entry);
	if (_entry->mouseEnterEvent != nullptr)
		entries[4].emplace_back(_entry);
	if (_entry->mouseLeftEvent != nullptr)
		entries[5].emplace_back(_entry);

	if (_entry->mouseButtonPressEvent != nullptr)
		entries[6].emplace_back(_entry);
	if (_entry->mouseButtonReleaseEvent != nullptr)
		entries[7].emplace_back(_entry);
	if (_entry->mouseWheelScrollEvent != nullptr)
		entries[8].emplace_back(_entry);
	if (_entry->joystickConnectEvent != nullptr)
		entries[9].emplace_back(_entry);
	if (_entry->joystickDisconnectEvent != nullptr)
		entries[10].emplace_back(_entry);

	if (_entry->joystickMoveEvent != nullptr)
		entries[11].emplace_back(_entry);
	if (_entry->joystickButtonPressEvent != nullptr)
		entries[12].emplace_back(_entry);
	if (_entry->joystickButtonReleaseEvent != nullptr)
		entries[13].emplace_back(_entry);
	if (_entry->touchBeginEvent != nullptr)
		entries[14].emplace_back(_entry);
	if (_entry->touchEndEvent != nullptr)
		entries[15].emplace_back(_entry);

	if (_entry->touchMoveEvent != nullptr)
		entries[16].emplace_back(_entry);
	if (_entry->closeEvent != nullptr)
		entries[17].emplace_back(_entry);
	if (_entry->resizeEvent != nullptr)
		entries[18].emplace_back(_entry);
	if (_entry->lostFocusEvent != nullptr)
		entries[19].emplace_back(_entry);
	if (_entry->gainedFocusEvent != nullptr)
		entries[20].emplace_back(_entry);

	update();
}

void InputMultiplexer::remove(InputEntry* _entry) {
	for (int i = 0; i <= 21; ++i) {
		auto& o = entries[i];
		for (auto it = o.begin(); it < o.end(); ++it)
			if (*it == _entry) {
				o.erase(it);
				break;
			}
	}
}

bool InputMultiplexer::fire(sf::Event& _event) {
	switch (_event.type) {
	case sf::Event::KeyPressed:
	{
		auto& e = entries[0];
		for (auto o : e) {
			if (o == nullptr) continue;
			if (o->keyPressEvent(_event.key.code, _event.key.alt, _event.key.control, _event.key.shift, _event.key.system))
				if (o->interrupting) break;
		}
	}
	return true;
	case sf::Event::KeyReleased:
	{
		auto& e = entries[1];
		for (auto o : e) {
			if (o == nullptr) continue;
			if (o->keyReleaseEvent(_event.key.code, _event.key.alt, _event.key.control, _event.key.shift, _event.key.system))
				if (o->interrupting) break;
		}
	}
	return true;
	case sf::Event::TextEntered:
	{
		auto& e = entries[2];
		for (auto o : e) {
			if (o == nullptr) continue;
			if (o->textEvent(_event.text.unicode))
				if (o->interrupting) break;
		}
	}
	return true;
	case sf::Event::MouseMoved:
	{
		auto& e = entries[3];
		int mY = MainStruct::get()->canvasHeight - _event.mouseMove.y;
		for (auto o : e) {
			if (o == nullptr) continue;
			if (o->mouseMoveEvent(_event.mouseMove.x, mY))
				if (o->interrupting) break;
		}
	}
	return true;
	case sf::Event::MouseEntered:
	{
		auto& e = entries[4];
		for (auto o : e) {
			if (o == nullptr) continue;
			if (o->mouseEnterEvent())
				if (o->interrupting) break;
		}
	}
	return true;
	case sf::Event::MouseLeft:
	{
		auto& e = entries[5];
		for (auto o : e) {
			if (o == nullptr) continue;
			if (o->mouseLeftEvent())
				if (o->interrupting) break;
		}
	}
	return true;
	case sf::Event::MouseButtonPressed:
	{
		auto& e = entries[6];
		int mY = MainStruct::get()->canvasHeight - _event.mouseButton.y;
		for (auto o : e) {
			if (o == nullptr) continue;
			if (o->mouseButtonPressEvent(_event.mouseButton.button, _event.mouseButton.x, mY))
				if (o->interrupting) break;
		}
	}
	return true;
	case sf::Event::MouseButtonReleased:
	{
		auto& e = entries[7];
		int mY = MainStruct::get()->canvasHeight - _event.mouseButton.y;
		for (auto o : e) {
			if (o == nullptr) continue;
			if (o->mouseButtonReleaseEvent(_event.mouseButton.button, _event.mouseButton.x, mY))
				if (o->interrupting) break;
		}
	}
	return true;
	case sf::Event::MouseWheelScrolled:
	{
		auto& e = entries[8];
		for (auto o : e) {
			if (o == nullptr) continue;
			if (o->mouseWheelScrollEvent(_event.mouseWheelScroll.wheel, _event.mouseWheelScroll.delta, _event.mouseWheelScroll.x, _event.mouseWheelScroll.y))
				if (o->interrupting) break;
		}
	}
	return true;
	case sf::Event::JoystickConnected:
	{
		auto& e = entries[9];
		for (auto o : e) {
			if (o == nullptr) continue;
			if (o->joystickConnectEvent(_event.joystickConnect.joystickId))
				if (o->interrupting) break;
		}
	}
	return true;
	case sf::Event::JoystickDisconnected:
	{
		auto& e = entries[10];
		for (auto o : e) {
			if (o == nullptr) continue;
			if (o->joystickConnectEvent(_event.joystickConnect.joystickId))
				if (o->interrupting) break;
		}
	}
	return true;
	case sf::Event::JoystickMoved:
	{
		auto& e = entries[11];
		for (auto o : e) {
			if (o == nullptr) continue;
			if (o->joystickMoveEvent(_event.joystickMove.joystickId, _event.joystickMove.axis, _event.joystickMove.position))
				if (o->interrupting) break;
		}
	}
	return true;
	case sf::Event::JoystickButtonPressed:
	{
		auto& e = entries[12];
		for (auto o : e) {
			if (o->joystickButtonPressEvent(_event.joystickButton.joystickId, _event.joystickButton.button))
				if (o->interrupting) break;
		}
	}
	return true;
	case sf::Event::JoystickButtonReleased:
	{
		auto& e = entries[13];
		for (auto o : e) {
			if (o->joystickButtonReleaseEvent(_event.joystickButton.joystickId, _event.joystickButton.button))
				if (o->interrupting) break;
		}
	}
	return true;
	case sf::Event::TouchBegan:
	{
		auto& e = entries[14];
		for (auto o : e) {
			if (o->touchBeginEvent(_event.touch.finger, _event.touch.x, _event.touch.y))
				if (o->interrupting) break;
		}
	}
	return true;
	case sf::Event::TouchEnded:
	{
		auto& e = entries[15];
		for (auto o : e) {
			if (o->touchEndEvent(_event.touch.finger, _event.touch.x, _event.touch.y))
				if (o->interrupting) break;
		}
	}
	return true;
	case sf::Event::TouchMoved:
	{
		auto& e = entries[16];
		for (auto o : e) {
			if (o->touchMoveEvent(_event.touch.finger, _event.touch.x, _event.touch.y))
				if (o->interrupting) break;
		}
	}
	case sf::Event::Closed:
	{
		auto& e = entries[17];
		for (auto o : e) {
			if (o == nullptr) continue;
			o->closeEvent();
		}
	}
	break;
	case sf::Event::Resized:
	{
		auto& e = entries[18];
		for (auto o : e) {
			if (o == nullptr) continue;
			o->resizeEvent(_event.size.width, _event.size.height);
		}
	}
	break;
	case sf::Event::LostFocus:
	{
		auto& e = entries[19];
		for (auto o : e) {
			if (o == nullptr) continue;
			o->lostFocusEvent();
		}
	}
	break;
	case sf::Event::GainedFocus:
	{
		auto& e = entries[20];
		for (auto o : e) {
			if (o == nullptr) continue;
			o->gainedFocusEvent();
		}
	}
	break;
	return true;
	}
	return false;
}