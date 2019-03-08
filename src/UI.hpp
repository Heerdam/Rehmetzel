#pragma once

#include <vector>
#include <algorithm>
#include <functional>

#include "MainStruct.hpp"
#include "InputMultiplexer.hpp"

namespace Heerbann {
	namespace UI {

		class Actor {
		protected:
			std::vector<Actor*> children;
			sf::IntRect aabb;
			Actor* parent;
		public:
			virtual void layout() = 0;
			virtual void act(float) = 0;
			virtual void draw(sf::RenderWindow& _window) = 0;

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
			sf::IntRect aabb;
		public:
			Stage() {
				InputMultiplexer::InputEntry* entry = new InputMultiplexer::InputEntry();
				entry->mouseButtonPressEvent = [&](sf::Mouse::Button _button, int _x, int _y)->bool {
					if (aabb.contains(sf::Vector2i(_x, _y)))
						return root->mouseButtonPressEvent(_button, _x, _y);
					return false;
				};

				entry->mouseButtonReleaseEvent = [&](sf::Mouse::Button _button, int _x, int _y)->bool {
					if (aabb.contains(sf::Vector2i(_x, _y)))
						return root->mouseButtonReleaseEvent(_button, _x, _y);
					return false;
				};

				entry->mouseMoveEvent = [&](int _x, int _y)->bool {
					if (aabb.contains(sf::Vector2i(_x, _y)))
						return root->mouseMoveEvent(_x, _y);
					return false;
				};
			};

			void add(Actor* _actor) {
				root->add(_actor);
			};

			void act(float _deltaTime) {
				if (root != nullptr) root->act(_deltaTime);
			};

			void draw(sf::RenderWindow& _window) {
				if (root != nullptr) root->draw(_window);
			};

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

		private:
			ButtonState state = ButtonState::up;

		public:

			void layout() override {
				for (auto c : children)
					c->layout();
			};

			void act(float _deltaTime) override {
				for (auto c : children)
					c->act(_deltaTime);
			};

			void draw(sf::RenderWindow& _window) override {
				for (auto c : children)
					c->draw(_window);
			};

			bool mouseMoveEvent(int _x, int _y) {

				bool isOver = aabb.contains(sf::Vector2i(_x, _y));
				
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
				if (aabb.contains(sf::Vector2i(_x, _y))) {
					for (auto c : children) {
						if (c->mouseButtonPressEvent(_button, _x, _y))
							return true;
					}
				}
				return false;
			};

			bool mouseButtonReleaseEvent(sf::Mouse::Button _button, int _x, int _y) {
				if (aabb.contains(sf::Vector2i(_x, _y))) {
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

			void layout() override {

				for (auto c : children)
					c->layout();
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

			bool mouseMoveEvent(int _x, int _y) {
				if (aabb.contains(sf::Vector2i(_x, _y))) {
					for (auto c : children) {
						if (c->mouseMoveEvent(_x, _y))
							return true;
					}
				}
				return false;
			};

			bool mouseButtonPressEvent(sf::Mouse::Button _button, int _x, int _y) {
				if (aabb.contains(sf::Vector2i(_x, _y))) {
					for (auto c : children) {
						if (c->mouseButtonPressEvent(_button, _x, _y))
							return true;
					}
				}
				return false;
			};

			bool mouseButtonReleaseEvent(sf::Mouse::Button _button, int _x, int _y) {
				if (aabb.contains(sf::Vector2i(_x, _y))) {
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
			void layout() override {
				for (auto c : children)
					c->layout();
			};

			void act(float _deltaTime) override {
				for (auto c : children)
					c->act(_deltaTime);
			};

			void draw(sf::RenderWindow& _window) override {
				for (auto c : children)
					c->draw(_window);
			};

			bool mouseMoveEvent(int _x, int _y) {
				if (aabb.contains(sf::Vector2i(_x, _y))) {
					for (auto c : children) {
						if (c->mouseMoveEvent(_x, _y))
							return true;
					}
				}
				return false;
			};

			bool mouseButtonPressEvent(sf::Mouse::Button _button, int _x, int _y) {
				if (aabb.contains(sf::Vector2i(_x, _y))) {
					for (auto c : children) {
						if (c->mouseButtonPressEvent(_button, _x, _y))
							return true;
					}
				}
				return false;
			};

			bool mouseButtonReleaseEvent(sf::Mouse::Button _button, int _x, int _y) {
				if (aabb.contains(sf::Vector2i(_x, _y))) {
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

