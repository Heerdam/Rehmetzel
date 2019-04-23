#pragma once

#include "MainStruct.hpp"

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

	class Box2dRenderer : public b2QueryCallback {

		std::vector<WorldObject*> objects;

	public:
		void draw(float, sf::RenderWindow&);
		bool ReportFixture(b2Fixture*) override;
	};

	class Camera {

		Ray* ray;
	
	public:

		Camera(const float, const float);

		Vec3 position;
		Vec3 direction;
		Vec3 up;
		Vec3 right;

		Mat4 projection;
		Mat4 view;
		Mat4 combined;
		Mat4 invProjectionView;

		float nearPlane = 1.0f;
		float farPlane = 100.f;
		float viewportWidth = 0.f;
		float viewportHeight = 0.f;

		Frustum* frustum;

		virtual void update() = 0;
		virtual void update(const bool) = 0;

		void lookAt(const float, const float, const float);
		void lookAt(const Vec3&);
		void normalizeUp();
		void normalizeUpYLocked();

		Quat getRotation(const Quat&);
		Mat4 getAsMat();
		
		void rotate(const float, const float, const float, const float);
		void rotate(const Vec3&, const float);
		void rotate(const Mat4&);
		void rotate(const Quat&);
		
		void rotateAround(const Vec3&, const Vec3&, float);
		void arcball(const Vec3&, const float, const float, const float);

		void transform(const Mat4&);

		void translate(const float, const float, const float);
		void translate(const Vec3&);

		Vec3 unproject(const Vec3&, const float, const float, const float, const float);
		Vec3 unproject(const Vec3&);

		Vec3 project(const Vec3&, const float, const float, const float, const float);
		Vec3 project(const Vec3&);

		const Ray* getPickRay(const float, const float, const float, const float, const float, const float);
		const Ray* getPickRay(const float, const float);
	};

	class OrthographicCamera : public Camera {
	public:

		float zoom = 1.f;

		OrthographicCamera();
		OrthographicCamera(const float, const float);

		void update() override;
		void update(const bool) override;

		void setToOrtho(const bool);
		void setToOrtho(const bool, const float, const float);
		void rotate(const float);
		void translate(const float, const float);
		void translate(const Vec2&);
	};

	class PerspectiveCamera : public Camera {
	public:
		float fieldOfView = 67.f;

		PerspectiveCamera();
		PerspectiveCamera(const float, const float, const float);

		void update () override;
		void update (const bool) override;
	};

	class AxisWidgetCamera : public Camera {

		//x - red
		//y - green (dir)
		//z - blue (up)

		ShaderProgram* shader;
		GLuint camLocation;
		GLuint vao;

		Camera* cam;
		Mat4 transform;

	public:
		AxisWidgetCamera(Camera*);

		void update() override;
		void update(const bool) override;
	};
}
