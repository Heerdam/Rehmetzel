#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	class SpriteBatch;
	class BoundingBox2f;
	class Viewport;
	
	namespace Text {
		class FontCache;
		struct StaticTextBlock;
		enum Align : int;
	}

	namespace UI {

		class Actor;

		class Stage {

		public:
			std::vector<Actor*> children;

			Stage() {};
			void act();
			void draw(SpriteBatch*);

		};

		class Actor {

		protected:
			sf::Vector2f position;

		public:
		
			virtual void act() {};
			virtual void draw(SpriteBatch*) {};
			virtual const sf::Vector2f& getPosition();
		};

		class Label : public Actor {

		};

		class StaticLabel : public Actor {

			Text::StaticTextBlock* block;

		public:
			StaticLabel(std::string, std::wstring, float, Text::Align);
			bool isLoaded();
			void setPosition(sf::Vector2f);
			void draw(SpriteBatch*) override;
		};

	}
}

