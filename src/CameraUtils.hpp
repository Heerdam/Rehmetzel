#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	using namespace Heerbann;

	enum ViewType {
		pers, ortho, ortho2d
	};

	class ViewportHandler {

		std::unordered_map<std::string, View*> views;

		Vec4ui currentGLBounds;

	public:

		bool checkBounds(const Vec4ui&);

		View* create(std::string, ViewType);
		void remove(std::string);

		View* operator[](std::string);

	};

	// glviewport
	// wrapper around a camera
	// pan/arcball
	// debug border
	// no camera without viewport
	// all viewport held in some handler

    struct View {

		sf::Mouse::Button panButton = sf::Mouse::Button::Right;
		float panXModifier = 0.1f;
		float panYModifier = 0.1f;
		float zoomModifier = 0.1f;
		Vec2 zoomBounds; 

		Vec4ui GLBounds;

		void setInteractive(bool);		

		const std::string id;
		const ViewType type;

		View(std::string, ViewType, ViewportHandler*);
		~View();

		void setViewportSize(uint, uint);
		void setViewportPosition(uint, uint);
		void setViewportBounds(uint, uint, uint, uint);

		void clear(const sf::Color);
		void drawDebug();
        void apply();

		Camera* getCamera();

		float* combined();

	private:		
		bool buttonPressed = false;
		bool inputsActive = false;
		Vec2i lastPos;

		ViewportHandler* parent;
		Camera* camera;
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

	struct OrthoPanJob {
		std::function<void(void*)> func;
		OrthographicCamera* cam;
		Vec2 dir;
		float speed;
		float t = 0.f;
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

	class ArcballCamera : public PerspectiveCamera {
		
	public:

		Vec3 target;
		float azimuth, height, distance;

		ArcballCamera();
		ArcballCamera(const float, const float, const float);
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
