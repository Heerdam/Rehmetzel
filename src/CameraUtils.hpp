#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

    class Viewport{

        sf::View cam;

    public:

        bool mousePressed = false;
        sf::Vector3f old, delta;

        bool mouseMoved(int _x, int _y){
            return false;
        };

        bool mouseDown(sf::Mouse::Button _button, int _x, int _y){
			mousePressed = _button == sf::Mouse::Button::Right ? true : mousePressed;
            return false;
        };

        bool mouseUp(sf::Mouse::Button _button, int _x, int _y) {
			mousePressed = _button == sf::Mouse::Button::Right ? false : mousePressed;
            return false;
        };

        Viewport(sf::String _id, int _prio){
            auto input = MainStruct::get()->inputListener;
            InputMultiplexer::InputEntry* entry = new InputMultiplexer::InputEntry();
            //entry->mouseMoveEvent = mouseMoved;
            //entry->mouseButtonPressEvent = mouseDown;
            //entry->mouseButtonReleaseEvent = mouseUp;
            //input->add(_id, entry);
        };

        void apply(sf::RenderWindow& _window, float _deltaTime){
            _window.setView(cam);
        };

       

    };

}