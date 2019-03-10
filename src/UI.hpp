#pragma once

#include <vector>
#include <algorithm>
#include <functional>

#include "MainStruct.hpp"
#include "InputMultiplexer.hpp"
#include "Utils.hpp"

namespace Heerbann {
	namespace UI {

		class Actor {
		protected:
			std::vector<Actor*> children;
			BoundingBox2f aabb;
			Actor* parent;
			sf::Vector2i position; //relative to parent
			bool isDirty = true;

		public:
			virtual void layout(sf::Vector2i _parent) {
				auto pos = _parent + position;
				for (auto c : children)
					c->layout(pos);
			};

			virtual void act(float _deltaTime) {
				for (auto c : children)
					c->act(_deltaTime);
			};

			virtual void draw(sf::RenderWindow& _window) {
				for (auto c : children)
					c->draw(_window);
			};

			virtual void getAABB(BoundingBox2f& _aabb) {
				aabb += _aabb;
				for (auto c : children)
					c->getAABB(aabb);
			}

			virtual bool mouseMoveEvent(int _x, int _y) {
				if (aabb == sf::Vector2i(_x, _y)) {
					for (auto c : children) {
						if (c->mouseMoveEvent(_x, _y))
							return true;
					}
				}
				return false;
			};

			virtual bool mouseButtonPressEvent(sf::Mouse::Button _button, int _x, int _y) {
				if (aabb == sf::Vector2i(_x, _y)) {
					for (auto c : children) {
						if (c->mouseButtonPressEvent(_button, _x, _y))
							return true;
					}
				}
				return false;
			};

			virtual bool mouseButtonReleaseEvent(sf::Mouse::Button _button, int _x, int _y) {
				if (aabb == sf::Vector2i(_x, _y)) {
					for (auto c : children) {
						if (c->mouseButtonReleaseEvent(_button, _x, _y))
							return true;
					}
				}
				return false;
			};

			void add(Actor* _actor) {
				children.emplace_back(_actor);
			};

			void remove() {
				for (auto it = parent->children.begin(); it != parent->children.end(); ++it) {
					if (*it == this) {
						parent->children.erase(it);
						parent = nullptr;
						return;
					}
				}
			}
		};

		class Stage {
			Actor* root = new Actor();
			BoundingBox2f aabb;
		public:
			Stage() {
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
			};

			void add(Actor* _actor) {
				root->add(_actor);
			};

			void layout() {
				root->layout(sf::Vector2i(0, 0));
			}

			void act(float _deltaTime) {
				root->act(_deltaTime);
			};

			void draw(sf::RenderWindow& _window) {
				root->draw(_window);
			};

			const BoundingBox2f& getAABB() {
				aabb.clr();
				root->getAABB(aabb);
				return aabb;
			}

		};

		class Button : public Actor {

		public:

			enum ButtonState {
				inactive, up, pressed, hover
			};

			//gets called when the state of the button changes
			std::function<void(ButtonState _state)> stateListener;
			//convenience listener for when the button gets called when clicked
			std::function<void()> clickListener;

			sf::Sprite *s_up, *s_pressed, *s_hover, *s_inactive;

		private:
			ButtonState state = ButtonState::up;

		public:

			virtual void layout(sf::Vector2i _parent) override {
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
			};

			virtual void draw(sf::RenderWindow& _window) {
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
			};

			bool mouseMoveEvent(int _x, int _y) {

				bool isOver = aabb == sf::Vector2i(_x, _y);
				
				if (isOver && state == ButtonState::up) {					
					state = ButtonState::hover; 
					if (stateListener != nullptr)
						stateListener(state);
				} else if (!isOver && state == ButtonState::hover) {
					state = ButtonState::up;
					if (stateListener != nullptr)
						stateListener(state);
				}

				if (isOver) {
					return Actor::mouseMoveEvent(_x, _y);
				}
				return false;
			};

			bool mouseButtonPressEvent(sf::Mouse::Button _button, int _x, int _y) {
				
				return Actor::mouseButtonPressEvent(_button, _y, _y);
			};

			bool mouseButtonReleaseEvent(sf::Mouse::Button _button, int _x, int _y) {
				
				return Actor::mouseButtonReleaseEvent(_button, _y, _y);
			};
		};

		class Label : public Actor {

		public:
			sf::Text text;

			Label(std::string _text, sf::Font* _font) : Actor() {
				text.setFont(*_font);
				text.setString(_text.c_str());
			};

			virtual void layout(sf::Vector2i _parent) override {
				auto pos = _parent + position;
				text.setPosition(sf::Vector2f(pos));
				auto bounds = text.getGlobalBounds();
				aabb.min = sf::Vector2f(position);
				aabb.max = sf::Vector2f(position) + sf::Vector2f(bounds.width, bounds.height);
				Actor::layout(_parent);
			};

		};

		class TextButton : public Button{
		public:
			Label* label;
		};

		class ProgressBar : public Actor {

			float value = 0.5f;

		public:

			sf::Sprite* background;
			sf::Sprite* bar;
			sf::Sprite* border;

			void setValue(float _val) {
				value = std::clamp(_val, 0.f, 1.f);
			};

			float getValue() {
				return value;
			};

			virtual void layout(sf::Vector2i _parent) override {

			};

			void act(float _deltaTime) {

			};

			void draw(sf::RenderWindow& _window) override {
				if (background != nullptr)
					_window.draw(*background);
				if (bar != nullptr)
					_window.draw(*bar);
				if (border != nullptr)
					_window.draw(*border);
				for (auto c : children)
					c->draw(_window);
				Actor::draw(_window);
			};

		};

		class Image : public Actor {
		public:
			sf::Sprite* img;

			virtual void layout(sf::Vector2i _parent) override {
				if (img != nullptr) {
					img->setPosition(sf::Vector2f(_parent) + sf::Vector2f(position));
					auto bounds = img->getGlobalBounds();
					aabb.min = sf::Vector2f(position);
					aabb.max = sf::Vector2f(position) + sf::Vector2f(bounds.width, bounds.height);
				}
				Actor::layout(_parent);
			};

			virtual void draw(sf::RenderWindow& _window) override {
				if (img != nullptr) _window.draw(*img);
				Actor::draw(_window);
			};

		};

	}
}

