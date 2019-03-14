
#include "CameraUtils.hpp"

using namespace Heerbann;

void Viewport::setBounds(int _x, int _y, int _width, int _height) {
	//float fx = 1.f / Main::width();
	//float fy = 1.f / Main::height();
	cam.setSize(sf::Vector2f((float)_width, (float)_height));
	//cam.setViewport(sf::FloatRect(fx * _x, fy * _y, fx * _width, fy * _height));
}

void Viewport::setSize(int _width, int _height) {
	int maxWidth = Main::width() - posX;
	int maxHeight = Main::height() - posY;
	width = std::clamp(_width, 2 * border, maxWidth);
	height = std::clamp(_height, border + topBorder, maxHeight);

	setBounds(posX, posY, width, height);
};

void Viewport::setPosition(int _x, int _y) {
	posX = std::clamp(_x, 0, (int)Main::width());
	posY = std::clamp(_y, 0, (int)Main::height());
	setSize(width, height);
	setBounds(posX, posY, width, height);
};

Viewport::Viewport(std::string _id, int _prio) {
	cam.setSize(sf::Vector2f((float)width, (float)height));
	cam.setCenter(sf::Vector2f(0.f, 0.f));
	InputEntry* entry = new InputEntry();
	entry->priority = _prio;

	entry->mouseMoveEvent = [&](int _x, int _y)->bool {
		int bounds = interactive ? inBounds(_x, _y) : 0;
		//std::cout << bounds << std::endl;
		if (mouseRightPressed) { //touch dragged
			sf::Vector2i delta(_x, _y);
			delta -= last;
			delta.x = (int)((float)delta.x * -1 * zoom);
			delta.y = (int)((float)delta.y * zoom);
			cam.move(sf::Vector2f(delta));
			last.x = _x;
			last.y = _y;
			return true;
		} else if (resizing || (mouseLeftPressed && bounds > 0)) {
			if (resizing == false) {
				borderRes = bounds;
				resizing = true;
			}
			sf::Vector2i delta(_x, _y);
			delta -= last;
			//std::cout << delta.x << " " << delta.y << std::endl;
			//delta.x = (int)((float)delta.x * - 1 * zoom);
			//delta.y = (int)((float)delta.y * zoom);
			switch (borderRes) {
			case 1://left
			{
				int newPosX = posX + (int)delta.x;

				int newWidth = width - (int)delta.x;
				if (newPosX <= 0)
					newWidth = width + posX;

				int leftC = posX + width;
				if (leftC - newPosX <= 2 * border)
					newPosX = leftC - 2 * border;

				setSize(newWidth, height);
				setPosition(newPosX, posY);
			}
			break;
			case 2: //right
				setSize(width + (int)delta.x, height);
				break;
			case 3: //top (move whole window)
				setPosition(posX + (int)delta.x, posY + (int)delta.y);
				break;
			case 4: //bottom
			{
				int newPosY = posY + (int)delta.y;
				int newHeight = height - (int)delta.y;

				if (newPosY <= 0)
					newHeight = height + posY;

				int topC = posY + height;
				if (topC - newPosY <= border + topBorder) {
					newPosY = topC - (border + topBorder);
					newHeight = border + topBorder;
				}

				setSize(width, newHeight);
				setPosition(posX, newPosY);
			}
			break;
			case 5: //left top corner
				//setPosition(posX + (int)delta.x, posY);
				//setSize(width, height + (int)delta.y);
				break;
			case 6: //left bottom corner
				//setPosition(posX + (int)delta.x, posY);
				//setPosition(posX, posY + (int)delta.y);
				break;
			case 7: //right top corner
			{
				if (exitClick != nullptr) {
					exitClick(_x, _y);
					resizing = false;
					mouseLeftPressed = false;
					borderRes = 0;
				}
			}
			break;
			case 8: //right bottom corner
			{
				int newPosY = posY + (int)delta.y;
				int newHeight = height - (int)delta.y;

				if (newPosY <= 0)
					newHeight = height + posY;

				int topC = posY + height;
				if (topC - newPosY <= border + topBorder) {
					newPosY = topC - (border + topBorder);
					newHeight = border + topBorder;
				}

				setSize(width + (int)delta.x, newHeight);
				setPosition(posX, newPosY);
			}
			break;
			}
			last.x = _x;
			last.y = _y;
			return true;
		}
		//std::cout << last.x << " " << last.y << std::endl;
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

		if (_button == sf::Mouse::Button::Left) {
			resizing = false;
			mouseLeftPressed = false;
			borderRes = 0;
		}

		last.x = _x;
		last.y = _y;
		return false;
	};

	entry->mouseWheelScrollEvent = [&](sf::Mouse::Wheel, float _delta, int, int)->bool {	
		zoom += zoomSpeed * (_delta < 0 ? 1 : -1);
		if(zoom > 0.1f && zoom < 100) cam.zoom(1 + zoomSpeed * (_delta < 0 ? 1 : -1));
		else zoom = std::clamp(zoom, 0.1f, 100.f);		
		return false;
	};

	Main::input_add(_id, entry);
 };

 void Viewport::apply(sf::RenderWindow& _window, float _deltaTime) {
	 
	 //glEnable(GL_SCISSOR_TEST);
	// glScissor(posX, posY, width, height);
	 if (clear) {
		 glClearColor(1.f / 255.f * clearColor.r, 1.f / 255.f * clearColor.g, 1.f / 255.f * clearColor.b, 1.f / 255.f * clearColor.a);
		 glClear(GL_COLOR_BUFFER_BIT);
	 }
	 //_window.setView(cam);
	 
	 glViewport(posX, posY, width, height);
	// glMatrixMode(GL_PROJECTION);
	// glLoadMatrixf(cam.getTransform().getMatrix());
	 //glMatrixMode(GL_MODELVIEW);

	 if (update != nullptr) update(_window, _deltaTime);
	 if (draw != nullptr) draw(_window, _deltaTime);
	 if (debugDraw) {
		 int mh = Main::height();
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
	 //glDisable(GL_SCISSOR_TEST);
 };