#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	using namespace Heerbann;

	struct WorldObject;

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

	class Box2dRenderer : public b2QueryCallback {

		std::vector<WorldObject*> objects;

	public:
		void draw(float, sf::RenderWindow&);
		bool ReportFixture(b2Fixture*) override;
	};

	struct Frustum;
	struct Ray;
	class Quaternion;

	class Camera {

		Ray* ray;
		Matrix4* tmpMat;

	public:

		Camera(float, float);

		sf::Vector3f position;
		sf::Vector3f direction;
		sf::Vector3f up;

		Matrix4* projection;
		Matrix4* view;
		Matrix4* combined;
		Matrix4* invProjectionView;

		float nearPlane = 1.0f;
		float farPlane = 100.f;
		float viewportWidth = 0.f;
		float viewportHeight = 0.f;

		Frustum* frustum;

		virtual void update() = 0;
		virtual void update(bool) = 0;

		void lookAt(float, float, float);
		void lookAt(const sf::Vector3f&);
		void normalizeUp();
		
		void rotate(float, float, float, float);
		void rotate(const sf::Vector3f&, float);
		void rotate(Matrix4*);
		void rotate(Quaternion*);
		
		void rotateAround(const sf::Vector3f&, const sf::Vector3f&, float);

		void transform(Matrix4*);

		void translate(float, float, float);
		void translate(const sf::Vector3f&);

		sf::Vector3f unproject(sf::Vector3f&, float, float, float, float);
		sf::Vector3f unproject(sf::Vector3f&);

		sf::Vector3f project(sf::Vector3f&, float, float, float, float);
		sf::Vector3f project(sf::Vector3f&);

		const Ray* getPickRay(float, float, float, float, float, float);
		const Ray* getPickRay(float, float);
	};

	class OrthographicCamera : public Camera {
	public:

		float zoom = 1.f;

		OrthographicCamera();
		OrthographicCamera(float, float);

		void update() override;
		void update(bool) override;

		void setToOrtho(bool);
		void setToOrtho(bool, float, float);
		void rotate(float);
		void translate(float, float);
		void translate(sf::Vector2f);
	};

	class PerspectiveCamera : public Camera {
	public:
		float fieldOfView = 67;

		PerspectiveCamera();
		PerspectiveCamera(float, float, float);

		void update () override;
		void update (bool) override;
	};

}
