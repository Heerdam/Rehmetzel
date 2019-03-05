#ifndef CAMERAUTILS_H
#define CAMERAUTILS_H

#include <functional>
#include <algorithm>
#include <iostream>

#include "MainStruct.hpp"
#include "InputMultiplexer.hpp"

namespace Heerbann {

	using namespace Heerbann;

    class Viewport{

        sf::View cam;

		int posX = 0, posY = 0, width = MainStruct::get()->canvasWidth, height = MainStruct::get()->canvasHeight;
		float zoom = 1;

	public:
		bool interactive = false;
		int border = 5, topBorder = 10;

		bool mouseRightPressed = false;
		bool mouseLeftPressed = false;

		sf::Vector2i last;

		bool debugDraw = false;
		bool clear = true;
		sf::Color clearColor = sf::Color::White;

		bool isLeft(int _x, int _y) {
			return (_x >= posX && _x <= posX + border);
		};

		bool isRight(int _x, int _y) {
			return (posX + width - border <= _x && _x <= posX + width);
		};

		bool isTop(int _x, int _y) {
			return (posY + height - topBorder <= _y && _y <= posY + height);
		};

		bool isBottom(int _x, int _y) {
			return (_y >= posY && _y <= posY + border);
		};

		int inBounds(int _x, int _y) {

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

		void setBounds(int _x, int _y, int _width, int _height) {
			float fx = 1.f / MainStruct::get()->canvasWidth;
			float fy = 1.f / MainStruct::get()->canvasHeight;
			cam.setViewport(sf::FloatRect(fx * _x, fy * _y, fx * _width, fy * _height));
		}

    public:
		void setSize(int _width, int _height) {
			int maxWidth = MainStruct::get()->canvasWidth - posX;
			int maxHeight = MainStruct::get()->canvasHeight - posY;
			width = std::clamp(_width, interactive ? 2 * border : 1, maxWidth); 
			height = std::clamp(_height, interactive ? 2 * border : 1, maxHeight); 

			setBounds(posX, posY, width, height);
		};

		void setPosition(int _x, int _y) {
			posX = std::clamp(_x, 0, MainStruct::get()->canvasWidth);
			posY = std::clamp(_y, 0, MainStruct::get()->canvasHeight);

			setBounds(posX, posY, width, height);
		};

		Viewport(std::string _id, int _prio) {

			InputMultiplexer::InputEntry* entry = new InputMultiplexer::InputEntry();
			entry->priority = _prio;

			entry->mouseMoveEvent = [&](int _x, int _y)->bool {
				int bounds = interactive ? inBounds(_x, _y) : 0;
				//std::cout << bounds << std::endl;
				if (mouseRightPressed) { //touch dragged
					sf::Vector2i delta(_x, _y);
					delta -= last;
					delta.x = (int)((float)delta.x -1 * zoom);
					delta.y = (int)((float)delta.x * zoom);
					cam.setCenter(cam.getCenter() + sf::Vector2f(delta));
					last.x = _x;
					last.y = _y;
					return true;
				} else if (mouseLeftPressed && bounds > 0) { //TODO völlige gugus
					sf::Vector2i delta(_x, _y);
					delta -= last;
					delta.x = (int)((float)delta.x - 1 * zoom);
					delta.y = (int)((float)delta.x * zoom);
					switch (bounds) {
						case 1://left
							setPosition(posX + (int)delta.x, posY);
							break;
						case 2: //right
							setSize(width + (int)delta.x, height);
							break;
						case 3: //top (move whole window)
							setPosition(posX + (int)delta.x, posY + (int)delta.y);
							setSize(width + (int)delta.x, height + (int)delta.y);
							break;
						case 4: //bottom
							setPosition(posX, posY + (int)delta.y);
							break;
						case 5: //left top corner
							setPosition(posX + (int)delta.x, posY);
							setSize(width, height + (int)delta.y);
							break;
						case 6: //left bottom corner
							setPosition(posX + (int)delta.x, posY);
							setPosition(posX, posY + (int)delta.y);
							break;
						case 7: //right top corner
							setSize(width + (int)delta.x, height);
							setSize(width, height + (int)delta.y);
							break;
						case 8: //right bottom corner
							setSize(width + (int)delta.x, height);
							setPosition(posX, posY + (int)delta.y);
							break;
					}
					last.x = _x;
					last.y = _y;
					return true;
				}
				last.x = _x;
				last.y = _y;
				return false;
			};

			entry->mouseButtonPressEvent = [&](sf::Mouse::Button _button, int _x, int _y)->bool {
				mouseRightPressed = _button == sf::Mouse::Button::Right ? true : mouseRightPressed;
				mouseLeftPressed = _button == sf::Mouse::Button::Left ? true : mouseLeftPressed;
				last.x = _x;
				last.y = _y;
				return false;
			};

			entry->mouseButtonReleaseEvent = [&](sf::Mouse::Button _button, int _x, int _y)->bool {
				mouseRightPressed = _button == sf::Mouse::Button::Right ? false : mouseRightPressed;
				mouseLeftPressed = _button == sf::Mouse::Button::Left ? true : mouseLeftPressed;
				last.x = _x;
				last.y = _y;
				return true;
			};

			entry->mouseWheelScrollEvent = [&](sf::Mouse::Wheel, float _delta, int, int)->bool {
				zoom = std::clamp(zoom += _delta, 0.1f, 100.f);
				cam.zoom(zoom);
				return false;
			};

			entry->resizeEvent = [&](int _width, int _height)->bool {
				return false;
			};
		
			MainStruct::get()->inputListener->add(_id, entry);
        };

        void apply(sf::RenderWindow& _window, float _deltaTime){
			glEnable(GL_SCISSOR_TEST);
			glScissor(posX, posY, width, height);
			if (clear) {				
				glClearColor(1.f / 255.f * clearColor.r, 1.f / 255.f * clearColor.g, 1.f / 255.f * clearColor.b, 1.f / 255.f * clearColor.a);
				glClear(GL_COLOR_BUFFER_BIT);				
			}
			          
			if (debugDraw) {
				int mh = MainStruct::get()->canvasHeight;
				sf::RectangleShape rec;
				rec.setFillColor(sf::Color::Blue);
				//left
				rec.setPosition(sf::Vector2f((float)posX, (float)(mh - posY - height)));
				rec.setSize(sf::Vector2f((float)border, (float)height));
				_window.draw(rec);
				//right
				rec.setPosition(sf::Vector2f((float)(posX + width - border), (float)(mh - posY - height)));
				rec.setSize(sf::Vector2f((float)border, (float)height));
				_window.draw(rec);
				//bottom
				rec.setPosition(sf::Vector2f((float)posX, (float)(mh - posY - border)));
				rec.setSize(sf::Vector2f((float)width, (float)border));
				_window.draw(rec);
				//top
				rec.setFillColor(sf::Color::Red);
				rec.setPosition(sf::Vector2f((float)posX, (float)(mh - posY - height)));
				rec.setSize(sf::Vector2f((float)width, (float)topBorder));
				_window.draw(rec);
			}
			_window.setView(cam);
			glDisable(GL_SCISSOR_TEST);
        };

    };

}

#endif