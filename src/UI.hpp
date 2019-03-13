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
			bool isDirty = true;

		public:
			sf::Vector2i position; //relative to parent

			virtual void layout(sf::Vector2i _parent);
			virtual void act(float _deltaTime);
			virtual void draw(sf::RenderWindow& _window);
			virtual void getAABB(BoundingBox2f& _aabb);
			virtual bool mouseMoveEvent(int _x, int _y);
			virtual bool mouseButtonPressEvent(sf::Mouse::Button _button, int _x, int _y);
			virtual bool mouseButtonReleaseEvent(sf::Mouse::Button _button, int _x, int _y);
			
			void add(Actor* _actor);
			void remove();
		};

		class Stage {
			Actor* root = new Actor();
			BoundingBox2f aabb;
		public:
			Stage();

			void add(Actor* _actor);
			void layout();
			void act(float _deltaTime);
			void draw(sf::RenderWindow& _window);
			const BoundingBox2f& getAABB();
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
			bool mouseLeftPressed = false;

		public:
			virtual void layout(sf::Vector2i _parent) override;
			virtual void draw(sf::RenderWindow& _window) override;
			virtual bool mouseMoveEvent(int _x, int _y) override;
			virtual bool mouseButtonPressEvent(sf::Mouse::Button _button, int _x, int _y) override;
			virtual bool mouseButtonReleaseEvent(sf::Mouse::Button _button, int _x, int _y) override;
		};

		class Label : public Actor {

		public:
			sf::Text text;

			Label(std::string _text, sf::Font* _font);

			virtual void layout(sf::Vector2i _parent) override;
			virtual void draw(sf::RenderWindow& _window);

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

			void setValue(float _val);
			float getValue();

			virtual void layout(sf::Vector2i _parent) override;
			virtual void act(float _deltaTime) override;
			virtual void draw(sf::RenderWindow& _window) override;
		};

		class Image : public Actor {
		public:
			sf::Sprite* img;

			virtual void layout(sf::Vector2i _parent) override;
			virtual void draw(sf::RenderWindow& _window) override;
		};

	}
}

