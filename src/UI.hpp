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
			sf::Vector2i relPos;
			bool isDirty = true;

		public:
			virtual void layout(sf::Vector2i) = 0;
			virtual void act(float) = 0;
			virtual void draw(sf::RenderWindow&) = 0;
			virtual void getAABB(BoundingBox2f&) = 0;

			virtual bool mouseMoveEvent(int, int) = 0;
			virtual bool mouseButtonPressEvent(sf::Mouse::Button, int, int) = 0;
			virtual bool mouseButtonReleaseEvent(sf::Mouse::Button, int, int) = 0;

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
			Actor* root;
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
				if (root != nullptr) root->layout(sf::Vector2i(0, 0));
			}

			void act(float _deltaTime) {
				if (root != nullptr) root->act(_deltaTime);
			};

			void draw(sf::RenderWindow& _window) {
				if (root != nullptr) root->draw(_window);
			};

			void getAABB() {
				if (root != nullptr) root->getAABB(aabb);
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

			void layout(sf::Vector2i _parent) override {
				if (s_up != nullptr) {
					auto bounds = s_up->getGlobalBounds();
					//relative aabb of sprite
					BoundingBox2f b(sf::Vector2f(bounds.left - _parent.x, bounds.top - _parent.y),
						sf::Vector2f(bounds.left - _parent.x + bounds.width, bounds.top - _parent.y + bounds.height));
					aabb += b;
				}
				if (s_pressed != nullptr) {
					auto bounds = s_pressed->getGlobalBounds();
					//relative aabb of sprite
					BoundingBox2f b(sf::Vector2f(bounds.left - _parent.x, bounds.top - _parent.y),
						sf::Vector2f(bounds.left - _parent.x + bounds.width, bounds.top - _parent.y + bounds.height));
					aabb += b;
				}
				if (s_hover != nullptr) {
					auto bounds = s_hover->getGlobalBounds();
					//relative aabb of sprite
					BoundingBox2f b(sf::Vector2f(bounds.left - _parent.x, bounds.top - _parent.y),
						sf::Vector2f(bounds.left - _parent.x + bounds.width, bounds.top - _parent.y + bounds.height));
					aabb += b;
				}
				if (s_inactive != nullptr) {
					auto bounds = s_inactive->getGlobalBounds();
					//relative aabb of sprite
					BoundingBox2f b(sf::Vector2f(bounds.left - _parent.x, bounds.top - _parent.y),
						sf::Vector2f(bounds.left - _parent.x + bounds.width, bounds.top - _parent.y + bounds.height));
					aabb += b;
				}
				for (auto c : children)
					c->layout(_parent + relPos);
			};

			void act(float _deltaTime) override {
				for (auto c : children)
					c->act(_deltaTime);
			};

			void draw(sf::RenderWindow& _window) override {
				for (auto c : children)
					c->draw(_window);
			};

			void getAABB(BoundingBox2f& _aabb) {
				aabb += _aabb;
				for (auto c : children)
					c->getAABB(aabb);
			}

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
					for (auto c : children) {
						if (c->mouseMoveEvent(_x, _y))
							return true;
					}
				}
				return false;
			};

			bool mouseButtonPressEvent(sf::Mouse::Button _button, int _x, int _y) {
				if (aabb == sf::Vector2i(_x, _y)) {
					for (auto c : children) {
						if (c->mouseButtonPressEvent(_button, _x, _y))
							return true;
					}
				}
				return false;
			};

			bool mouseButtonReleaseEvent(sf::Mouse::Button _button, int _x, int _y) {
				if (aabb == sf::Vector2i(_x, _y)) {
					for (auto c : children) {
						if (c->mouseButtonReleaseEvent(_button, _x, _y))
							return true;
					}
				}
				return false;
			};
		};

		class LoadingBar : public Actor {

			float value = 0.5f;

		public:

			bool isInteractive = false;

			sf::Sprite* background;
			sf::Sprite* bar;
			sf::Sprite* border;

			void setValue(float _val) {
				value = std::clamp(_val, 0.f, 1.f);
			};

			float getValue() {
				return value;
			};

			void layout(sf::Vector2i _parent) override {

				for (auto c : children)
					c->layout(_parent + relPos);
			};

			void act(float _deltaTime) override {
				for (auto c : children)
					c->act(_deltaTime);
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

			};

			void getAABB(BoundingBox2f& _aabb) {
				aabb += _aabb;
				for (auto c : children)
					c->getAABB(aabb);
			}

			bool mouseMoveEvent(int _x, int _y) {
				if (aabb == sf::Vector2i(_x, _y)) {
					for (auto c : children) {
						if (c->mouseMoveEvent(_x, _y))
							return true;
					}
				}
				return false;
			};

			bool mouseButtonPressEvent(sf::Mouse::Button _button, int _x, int _y) {
				if (aabb == sf::Vector2i(_x, _y)) {
					for (auto c : children) {
						if (c->mouseButtonPressEvent(_button, _x, _y))
							return true;
					}
				}
				return false;
			};

			bool mouseButtonReleaseEvent(sf::Mouse::Button _button, int _x, int _y) {
				if (aabb == sf::Vector2i(_x, _y)) {
					for (auto c : children) {
						if (c->mouseButtonReleaseEvent(_button, _x, _y))
							return true;
					}
				}
				return false;
			};

		};

		class Label : public Actor {

		public:
			sf::Text text;

			Label(std::string _text, sf::Font* _font) {
				text.setFont(*_font);
				text.setString(_text.c_str());
			};

			void layout(sf::Vector2i _parent) override {
				auto pos = _parent + relPos;
				text.setPosition(sf::Vector2f(pos));
				for (auto c : children)
					c->layout(pos);
			};

			void act(float _deltaTime) override {
				for (auto c : children)
					c->act(_deltaTime);
			};

			void draw(sf::RenderWindow& _window) override {
				_window.draw(text);
				for (auto c : children)
					c->draw(_window);
			};

			void getAABB(BoundingBox2f& _aabb) {
				aabb += _aabb;
				for (auto c : children)
					c->getAABB(aabb);
			}

			bool mouseMoveEvent(int _x, int _y) {
				if (aabb == sf::Vector2i(_x, _y)) {
					for (auto c : children) {
						if (c->mouseMoveEvent(_x, _y))
							return true;
					}
				}
				return false;
			};

			bool mouseButtonPressEvent(sf::Mouse::Button _button, int _x, int _y) {
				if (aabb == sf::Vector2i(_x, _y)) {
					for (auto c : children) {
						if (c->mouseButtonPressEvent(_button, _x, _y))
							return true;
					}
				}
				return false;
			};

			bool mouseButtonReleaseEvent(sf::Mouse::Button _button, int _x, int _y) {
				if (aabb == sf::Vector2i(_x, _y)) {
					for (auto c : children) {
						if (c->mouseButtonReleaseEvent(_button, _x, _y))
							return true;
					}
				}
				return false;
			};
		};

	}
}

