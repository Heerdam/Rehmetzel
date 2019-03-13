#pragma once

#include <functional>
#include <algorithm>
#include <iostream>

#include "MainStruct.hpp"
#include "InputMultiplexer.hpp"

namespace Heerbann {

	using namespace Heerbann;

    class Viewport{
	public:
		int posX = 0, posY = 0, width = 640, height = 480;

		sf::View cam;
		float zoom = 1.f;

		bool interactive = false;
		int border = 5, topBorder = 10;

		std::function<void(int _x, int _y)> exitClick;

		bool debugDraw = false;
		bool clear = true;
		sf::Color clearColor = sf::Color::White;

		float zoomSpeed = .1f;

	private:

		bool mouseRightPressed = false;
		bool mouseLeftPressed = false;

		bool resizing = false;
		int borderRes = 0;

		sf::Vector2i last;

		constexpr bool isLeft(int _x, int _y) {
			return (_x >= posX && _x <= posX + border);
		};

		constexpr bool isRight(int _x, int _y) {
			return (posX + width - border <= _x && _x <= posX + width);
		};

		constexpr bool isTop(int _x, int _y) {
			return (posY + height - topBorder <= _y && _y <= posY + height);
		};

		constexpr bool isBottom(int _x, int _y) {
			return (_y >= posY && _y <= posY + border);
		};

		constexpr int inBounds(int _x, int _y) {

			bool left = isLeft(_x, _y);
			bool right = isRight(_x, _y);
			bool top = isTop(_x, _y);
			bool bottom = isBottom(_x, _y);

			//left
			if (left && !top && !bottom) return 1;
			//right
			if (right && !top && !bottom) return 2;
			//top
			if (top && !left && !right) return 3;
			//bottom
			if (bottom && !left && !right) return 4;
			//left top corner
			if (left && top) return 5;
			//left bottom corner
			if (left && bottom) return 6;
			//right top corner
			if (right && top) return 7;
			//right bottom corner
			if (right && bottom) return 8;

			return 0;
		};

		void setBounds(int _x, int _y, int _width, int _height);

    public:
		void setSize(int, int);
		void setPosition(int, int);

		Viewport(std::string, int);

		std::function<void(sf::RenderWindow& _window, float _deltaTime)> update = nullptr;
		std::function<void(sf::RenderWindow& _window, float _deltaTime)> draw = nullptr;
        void apply(sf::RenderWindow& _window, float _deltaTime);

    };

}
