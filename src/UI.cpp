
#include "UI.hpp"

using namespace Heerbann;
using namespace UI;

void Actor::layout(sf::Vector2i _parent) {
	auto pos = _parent + position;
	for (auto c : children)
		c->layout(pos);
}

void Actor::act(float _deltaTime) {
	for (auto c : children)
		c->act(_deltaTime);
}

void Actor::draw(sf::RenderWindow& _window) {
	for (auto c : children)
		c->draw(_window);
}

void Actor::getAABB(BoundingBox2f& _aabb) {
	aabb += _aabb;
	for (auto c : children)
		c->getAABB(aabb);
}

bool Actor::mouseMoveEvent(int _x, int _y) {
	if (aabb == sf::Vector2i(_x, _y)) {
		for (auto c : children) {
			if (c->mouseMoveEvent(_x, _y))
				return true;
		}
	}
	return false;
}

bool Actor::mouseButtonPressEvent(sf::Mouse::Button _button, int _x, int _y) {
	if (aabb == sf::Vector2i(_x, _y)) {
		for (auto c : children) {
			if (c->mouseButtonPressEvent(_button, _x, _y))
				return true;
		}
	}
	return false;
}

bool Actor::mouseButtonReleaseEvent(sf::Mouse::Button _button, int _x, int _y) {
	if (aabb == sf::Vector2i(_x, _y)) {
		for (auto c : children) {
			if (c->mouseButtonReleaseEvent(_button, _x, _y))
				return true;
		}
	}
	return false;
}

void Actor::add(Actor* _actor) {
	children.emplace_back(_actor);
}

void Actor::remove() {
	for (auto it = parent->children.begin(); it != parent->children.end(); ++it) {
		if (*it == this) {
			parent->children.erase(it);
			parent = nullptr;
			return;
		}
	}
}

Stage::Stage() {
	InputMultiplexer::InputEntry* entry = new InputMultiplexer::InputEntry();
	entry->mouseButtonPressEvent = [&](sf::Mouse::Button _button, int _x, int _y)->bool {
		if (aabb == sf::Vector2i(_x, _y))
			return root->mouseButtonPressEvent(_button, _x, _y);
		return false;
	};

	entry->mouseButtonReleaseEvent = [&](sf::Mouse::Button _button, int _x, int _y)->bool {
		if (aabb == sf::Vector2i(_x, _y))
			return root->mouseButtonReleaseEvent(_button, _x, _y);
		return false;
	};

	entry->mouseMoveEvent = [&](int _x, int _y)->bool {
		if (aabb == sf::Vector2i(_x, _y))
			return root->mouseMoveEvent(_x, _y);
		return false;
	};
}

void Stage::add(Actor* _actor) {
	root->add(_actor);
}

void Stage::layout() {
	root->layout(sf::Vector2i(0, 0));
}

void Stage::act(float _deltaTime) {
	root->act(_deltaTime);
}

void Stage::draw(sf::RenderWindow& _window) {
	root->draw(_window);
}

const BoundingBox2f& Stage::getAABB() {
	aabb.clr();
	root->getAABB(aabb);
	return aabb;
}

void Button::layout(sf::Vector2i _parent) {
	if (s_up != nullptr) {
		s_up->setPosition(sf::Vector2f(_parent) + sf::Vector2f(position));
		auto bounds = s_up->getGlobalBounds();
		//relative aabb of sprite
		BoundingBox2f b(sf::Vector2f(bounds.left - _parent.x, bounds.top - _parent.y),
			sf::Vector2f(bounds.left - _parent.x + bounds.width, bounds.top - _parent.y + bounds.height));
		aabb += b;
	}
	if (s_pressed != nullptr) {
		s_pressed->setPosition(sf::Vector2f(_parent) + sf::Vector2f(position));
		auto bounds = s_pressed->getGlobalBounds();
		//relative aabb of sprite
		BoundingBox2f b(sf::Vector2f(bounds.left - _parent.x, bounds.top - _parent.y),
			sf::Vector2f(bounds.left - _parent.x + bounds.width, bounds.top - _parent.y + bounds.height));
		aabb += b;
	}
	if (s_hover != nullptr) {
		s_hover->setPosition(sf::Vector2f(_parent) + sf::Vector2f(position));
		auto bounds = s_hover->getGlobalBounds();
		//relative aabb of sprite
		BoundingBox2f b(sf::Vector2f(bounds.left - _parent.x, bounds.top - _parent.y),
			sf::Vector2f(bounds.left - _parent.x + bounds.width, bounds.top - _parent.y + bounds.height));
		aabb += b;
	}
	if (s_inactive != nullptr) {
		s_inactive->setPosition(sf::Vector2f(_parent) + sf::Vector2f(position));
		auto bounds = s_inactive->getGlobalBounds();
		//relative aabb of sprite
		BoundingBox2f b(sf::Vector2f(bounds.left - _parent.x, bounds.top - _parent.y),
			sf::Vector2f(bounds.left - _parent.x + bounds.width, bounds.top - _parent.y + bounds.height));
		aabb += b;
	}
	Actor::layout(_parent);
}

void Button::draw(sf::RenderWindow& _window) {
	switch (state) {
	case ButtonState::hover:
		if (s_hover != nullptr) _window.draw(*s_hover);
		else if (s_up != nullptr) _window.draw(*s_up);
		break;
	case ButtonState::inactive:
		if (s_inactive != nullptr) _window.draw(*s_inactive);
		else if (s_up != nullptr) _window.draw(*s_up);
		break;
	case ButtonState::pressed:
		if (s_pressed != nullptr) _window.draw(*s_pressed);
		else if (s_up != nullptr) _window.draw(*s_up);
		break;
	case ButtonState::up:
		if (s_up != nullptr) _window.draw(*s_up);
		break;
	}
	Actor::draw(_window);
}

bool Button::mouseMoveEvent(int _x, int _y) {

	bool isOver = aabb == sf::Vector2i(_x, _y);

	if (!mouseLeftPressed && state != ButtonState::inactive) {
		if (isOver && state == ButtonState::up) { //start hovering
			state = ButtonState::hover;
			if (stateListener != nullptr)
				stateListener(state);
		} else if (!isOver && state == ButtonState::hover) { //stop hovering
			state = ButtonState::up;
			if (stateListener != nullptr)
				stateListener(state);
		}
	} else if (mouseLeftPressed && state != ButtonState::inactive) {
		if (isOver && state == ButtonState::up || state == ButtonState::hover) {//change to pressed
			state = ButtonState::pressed;
			if (stateListener != nullptr)
				stateListener(state);
			if (clickListener != nullptr)
				clickListener();
		} else if (state == ButtonState::pressed) {//change to up
			state = isOver ? ButtonState::hover : ButtonState::up;
			if (stateListener != nullptr)
				stateListener(state);
		}
	}

	if (isOver) {
		return Actor::mouseMoveEvent(_x, _y);
	}
	return false;
}

bool Button::mouseButtonPressEvent(sf::Mouse::Button _button, int _x, int _y) {
	mouseLeftPressed = _button == sf::Mouse::Button::Left ? true : mouseLeftPressed;
	return Actor::mouseButtonPressEvent(_button, _y, _y);
}

bool Button::mouseButtonReleaseEvent(sf::Mouse::Button _button, int _x, int _y) {
	mouseLeftPressed = _button == sf::Mouse::Button::Right ? false : mouseLeftPressed;
	return Actor::mouseButtonReleaseEvent(_button, _y, _y);
}

Label::Label(std::string _text, sf::Font* _font) : Actor() {
	text.setFont(*_font);
	text.setString(_text.c_str());
}

void Label::layout(sf::Vector2i _parent) {
	auto pos = _parent + position;
	text.setPosition(sf::Vector2f(pos));
	auto bounds = text.getGlobalBounds();
	aabb.min = sf::Vector2f(position);
	aabb.max = sf::Vector2f(position) + sf::Vector2f(bounds.width, bounds.height);
	Actor::layout(_parent);
}

void ProgressBar::setValue(float _val) {
	value = std::clamp(_val, 0.f, 1.f);
}

float ProgressBar::getValue() {
	return value;
}

void ProgressBar::layout(sf::Vector2i _parent) {
	if (background != nullptr) {
		background->setPosition(sf::Vector2f(_parent) + sf::Vector2f(position));
		auto bounds = background->getGlobalBounds();
		aabb.min = sf::Vector2f(position);
		aabb.max = sf::Vector2f(position) + sf::Vector2f(bounds.width, bounds.height);
	}
	if (bar != nullptr) {
		bar->setPosition(sf::Vector2f(_parent) + sf::Vector2f(position));
		auto bounds = bar->getGlobalBounds();
		aabb.min = sf::Vector2f(position);
		aabb.max = sf::Vector2f(position) + sf::Vector2f(bounds.width, bounds.height);
	}
	if (border != nullptr) {
		border->setPosition(sf::Vector2f(_parent) + sf::Vector2f(position));
		auto bounds = border->getGlobalBounds();
		aabb.min = sf::Vector2f(position);
		aabb.max = sf::Vector2f(position) + sf::Vector2f(bounds.width, bounds.height);
	}
}

void ProgressBar::act(float _deltaTime) {
	if (bar != nullptr) {
		bar->setScale(sf::Vector2f(value, 1));
	}
}

void ProgressBar::draw(sf::RenderWindow& _window) {
	if (background != nullptr)
		_window.draw(*background);
	if (bar != nullptr)
		_window.draw(*bar);
	if (border != nullptr)
		_window.draw(*border);
	for (auto c : children)
		c->draw(_window);
	Actor::draw(_window);
}

void Image::layout(sf::Vector2i _parent) {
	if (img != nullptr) {
		img->setPosition(sf::Vector2f(_parent) + sf::Vector2f(position));
		auto bounds = img->getGlobalBounds();
		aabb.min = sf::Vector2f(position);
		aabb.max = sf::Vector2f(position) + sf::Vector2f(bounds.width, bounds.height);
	}
	Actor::layout(_parent);
}

void Image::draw(sf::RenderWindow& _window) {
	if (img != nullptr) _window.draw(*img);
	Actor::draw(_window);
}