#pragma once

#include <limits>

#include "MainStruct.hpp"

namespace Heerbann {

	bool almost_equal(float _f1, float _f2);

	class BoundingBox2f {

	public:
		sf::Vector2f min;
		sf::Vector2f max;
		sf::Vector2f cnt;
		sf::Vector2f dim;

		void set(const sf::Vector2f&, const sf::Vector2f&);
		void set(const BoundingBox2f&);
		void inf();
		void ext(const sf::Vector2f&);
		void clr();
		boolean isValid();
		void ext(const BoundingBox2f&);
		bool contains(const BoundingBox2f&);
		bool intersects(const BoundingBox2f&);
		bool contains(const sf::Vector2f&);
		void ext(float, float);
		BoundingBox2f();
		BoundingBox2f(sf::Vector2f, sf::Vector2f);

		bool operator==(const BoundingBox2f& _b) {
			return contains(_b);
		};

		bool operator==(const sf::Vector2f _v) {
			return contains(_v);
		};

		bool operator==(const sf::Vector2i _v) {
			return contains(sf::Vector2f(_v));
		};

		BoundingBox2f& operator+(const BoundingBox2f& _b) {
			ext(_b);
			return *this;
		}

		BoundingBox2f& operator+=(const BoundingBox2f& _b) {
			ext(_b);
			return *this;
		}

	};

}
