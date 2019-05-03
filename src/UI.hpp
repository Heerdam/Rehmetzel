#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	namespace UI {

		class Stage {

		public:
			std::vector<Actor*> children;

			Stage() {};
			void act();
			void draw(SpriteBatch*);

		};

		class Actor {

		protected:
			Vec2 position;

		public:
		
			virtual void act() {};
			virtual void draw(SpriteBatch*) {};
			virtual const Vec2& getPosition();
		};

		class Label : public Actor {

		};	

	}
}

