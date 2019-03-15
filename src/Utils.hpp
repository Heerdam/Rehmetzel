#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	bool almost_equal(float _f1, float _f2); //TODO

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
		bool isValid();
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

	class VAO {
	protected:
		GLuint vao, vbo;
		GLint cameraUniformHandle;

		float* data;

		int vertexCount, vertexSize;

	public:
		virtual void setData(sf::Shader*, float*, int);
		virtual void draw(sf::Shader*);

	};

	class BGVAO : public VAO {

		GLuint texLoc[9];
		sf::Texture* tex[9];

		GLint cameraUniformHandle;

	public:
		void setData(sf::Shader*, float*, int) override;
		void draw(sf::Shader*) override;
	};

	class DebugDraw : public b2Draw {
	public:
		DebugDraw();
		void DrawPolygon(const b2Vec2*, int32, const b2Color&);
		void DrawSolidPolygon(const b2Vec2*, int32, const b2Color&);
		void DrawCircle(const b2Vec2&, float32, const b2Color&);
		void DrawSolidCircle(const b2Vec2&, float32, const b2Vec2&, const b2Color&);
		void DrawSegment(const b2Vec2&, const b2Vec2&, const b2Color&);
		void DrawTransform(const b2Transform&);
		void DrawPoint(const b2Vec2&, float32, const b2Color&);
		void DrawString(int, int, const char*);
		void DrawAABB(b2AABB*, const b2Color&);
		sf::Color B2SFColor(const b2Color&, int);
		void DrawMouseJoint(b2Vec2&, b2Vec2&, const b2Color&, const b2Color&);
	};

}
