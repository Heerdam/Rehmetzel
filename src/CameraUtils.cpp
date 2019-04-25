
#include "CameraUtils.hpp"
#include "World.hpp"
#include "InputMultiplexer.hpp"
#include "Utils.hpp"
#include "Math.hpp"

using namespace Heerbann;

bool ViewportHandler::checkBounds(const Vec4ui& _bounds) {
	if (currentGLBounds == _bounds) return true;
	currentGLBounds[0] = _bounds[0];
	currentGLBounds[1] = _bounds[1];
	currentGLBounds[2] = _bounds[2];
	currentGLBounds[3] = _bounds[3];
	return false;
}

View* ViewportHandler::create(std::string _id, ViewType _type) {
	View* out = new View(_id, _type, this);
	views[_id] = out;
	return out;
}

void ViewportHandler::remove(std::string _id) {
	View* out = views[_id];
	delete out;
	views[_id] = nullptr;
}

View* ViewportHandler::operator[](std::string _id) {
	return views[_id];
}

void View::setViewportBounds(uint _x, uint _y, uint _width, uint _height) {
	GLBounds[0] = _x;
	GLBounds[1] = _y;
	GLBounds[2] = _width;
	GLBounds[3] = _height;
	camera->viewportWidth = _width;
	camera->viewportHeight = _height;
}

void View::clear(const sf::Color _color) {
	glClearColor(colF(_color.r), colF(_color.g), colF(_color.b), colF(_color.a));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void View::drawDebug() {
	//TODO
}

void View::setViewportSize(uint _width, uint _height) {
	setViewportBounds(GLBounds[0], GLBounds[1], _width, _height);
}

void View::setViewportPosition(uint _x, uint _y) {
	setViewportBounds(_x, _y, GLBounds[2], GLBounds[3]);
}

void View::setInteractive(bool _setActive) {
	if (_setActive && !inputsActive) {
		inputsActive = true;
		InputEntry* entry = new InputEntry();
		entry->mouseMoveEvent = [&](int _x, int _y)->bool {
			switch (type) {
				case ViewType::pers:
				{
					ArcballCamera* cam = reinterpret_cast<ArcballCamera*>(camera);
					cam->azimuth += (_x - lastPos.x) * panXModifier;
					cam->height += (_y - lastPos.y) * panYModifier;
					camera->arcball(cam->target, cam->azimuth, cam->height, cam->distance);
				}
				return true;
				case ViewType::ortho:
				{
				}
				return true;
				case ViewType::ortho2d:
				{
					OrthographicCamera* cam = reinterpret_cast<OrthographicCamera*>(camera);
					cam->translate(static_cast<float>(_x - lastPos.x), static_cast<float>(_y - lastPos.y));
				}
				return true;
			}		
			return false;
		};

		entry->mouseButtonPressEvent = [&](sf::Mouse::Button _button, int _x, int _y)->bool {
			buttonPressed = _button == panButton ? true : buttonPressed;
			lastPos = Vec2i(_x, _y);
			return false;
		};

		entry->mouseButtonReleaseEvent = [&](sf::Mouse::Button _button, int _x, int _y)->bool {
			buttonPressed = _button == panButton ? false : buttonPressed;		
			switch (type) {
				case ViewType::ortho2d:
				{
					OrthoPanJob* job = new OrthoPanJob();
					job->cam = reinterpret_cast<OrthographicCamera*>(camera);
					Vec2 delta = Vec2i(_x, _y) - lastPos;
					job->dir = NOR(delta);
					job->speed = LEN(delta);
					job->func = [&](void* _entry) {
						if (buttonPressed) {
							delete _entry;
							return;
						}
						auto job = reinterpret_cast<OrthoPanJob*>(_entry);
						job->cam->translate(INTERPOLATE((job->dir*job->speed), Vec2(0.f, 0.f), (job->t += M_Delta)));
						if (!(job->t > 1.f || EQUAL(job->t, 1.f)))
							M_Main->addJob(job->func, job);
						else delete _entry;
					};
					M_Main->addJob(job->func, job);
				}
				break;
			}
			lastPos = Vec2i(_x, _y);
			return false;
		};

		entry->mouseWheelScrollEvent = [&](sf::Mouse::Wheel, float _delta, int, int)->bool {
			switch (type) {
				case ViewType::pers:
				{
					ArcballCamera* cam = reinterpret_cast<ArcballCamera*>(camera);
					cam->distance = CLAMP(cam->distance += _delta * zoomModifier, zoomBounds[0], zoomBounds[1]);
					camera->arcball(cam->target, cam->azimuth, cam->height, cam->distance);
				}
				return true;
				case ViewType::ortho:
				{



				}
				return true;
				case ViewType::ortho2d:
				{
					OrthographicCamera* cam = reinterpret_cast<OrthographicCamera*>(camera);
					cam->zoom = CLAMP(cam->zoom += (_delta * zoomModifier), zoomBounds[0], zoomBounds[1]);
					//TODO zoom to mousepointer
				}
				return true;
			}
			return false;
		};

		M_Input->add(id, entry);

	} else if(inputsActive){
		inputsActive = false;
		M_Input->remove(id);
	}
	
}

View::View(std::string _id, ViewType _type, ViewportHandler* _parent) : parent(_parent), id(_id), type(_type) {
	switch (type) {
		case ViewType::pers:
		{
			ArcballCamera* cam = new ArcballCamera();
			setViewportBounds(0, 0, M_WIDTH, M_HEIGHT);
			camera = cam;
			zoomBounds = Vec2(1.f, 100.f);
			cam->distance = 50.f;
			camera->arcball(cam->target, cam->azimuth, cam->height, cam->distance);
		}
		break;
		case ViewType::ortho:
		{
		}
		break;
		case ViewType::ortho2d:
		{
			OrthographicCamera* cam = new OrthographicCamera();
			setViewportBounds(0, 0, M_WIDTH, M_HEIGHT);
			camera = cam;
			zoomBounds = Vec2(0.1f, 100.f);
			cam->setToOrtho(false);
		}
		break;
	}	
 }

View::~View() {
	setInteractive(false);
}

 void View::apply() {
	 	 
	 if(!parent->checkBounds(GLBounds))
		 glViewport(GLBounds[0], GLBounds[1], GLBounds[2], GLBounds[3]);

	 camera->viewportWidth = GLBounds[2];
	 camera->viewportHeight = GLBounds[3];

	 camera->update();

 }
 Camera* View::getCamera() {
	 return camera;
 }

 float* View::combined() {
	 return ToArray(camera->combined);
 }

 Camera::Camera(const float _viewportWidth, const float _viewportHeight) :viewportWidth(_viewportWidth), viewportHeight(_viewportHeight),
	 frustum(new Frustum()), ray(new Ray()){
	 direction = Vec3(0.f, 0.f, -1.f);
	 up = UVY;
 }

 void Camera::lookAt(const float _x, const float _y, const float _z) {
	 direction = NOR(Vec3(_x, _y, _z) - position);
	 view = LOOKAT(position, Vec3(_x, _y, _z), up);
	 normalizeUp();
 }

 void Camera::lookAt(const Vec3& _pos) {
	 lookAt(_pos.x, _pos.y, _pos.z);
 }

 void Camera::normalizeUp() {
	 right = NOR(CRS(direction, up));
	 up = NOR(CRS(right, direction));
 }

 void Camera::normalizeUpYLocked() {
	 right = NOR(CRS(UVY, direction));
	 up = NOR(CRS(right, direction));
 }

 Quat Camera::getRotation(const Quat& _quat) {	
	 return setFromAxes(direction, right, up);
 }

 Mat4 Camera::getAsMat() {
	 return Mat4();
 }

 void Camera::rotate(const float _axisX, const float _axisY, const float _axisZ, const float _angle) {
	 Mat4 rot = ROTATEMATRIX(Mat4(), _angle, Vec3(_axisX, _axisY, _axisZ));
	 direction *= rot;
	 up *= rot;
	 right *= rot;
 }

 void Camera::rotate(const Vec3& _axis, const float _angle) {
	 rotate(_axis.x, _axis.y, _axis.z, _angle);
 }

 void Camera::rotate(const Mat4& _transform) {	 
	 direction *= _transform;
	 up *= transform;
	 right *= _transform;
 }

 void Camera::rotate(const Quat& _quat) {
	 Mat4 rot = TOMAT4(_quat);
	 direction *= rot;
	 up *= rot;
	 right *= rot;
 }

 void Camera::rotateAround(const Vec3& _point, const Vec3& _axis, const float _angle) {
	 Vec3 tmpVec = _point - position;
	 translate(tmpVec);
	 rotate(_axis, _angle);
	 Mat4 rot = ROTATEMATRIX(Mat4(), _angle, _axis);
	 tmpVec *= rot;
	 translate(-tmpVec.x, -tmpVec.y, -tmpVec.z);
 }

 void Camera::arcball(const Vec3& _point, const float _azimuth, const float _altitude, const float _radius) {
	
	 float ahh = TORAD(_altitude);
	 float azz = TORAD(_azimuth);
	 float ah = SIN(ahh);
	 float az = COS(azz);

	 float x = _radius * SIN(TORAD(_altitude)) * COS(TORAD(_azimuth));
	 float y = _radius * SIN(TORAD(_altitude)) * SIN(TORAD(_azimuth));
	 float z = _radius * COS(TORAD(_altitude));

	 position.x = x;
	 position.y = z;
	 position.z = y;

	 lookAt(_point);
	
	 normalizeUpYLocked();
	 update();
 }

 void Camera::transform(const Mat4& _transform) {
	 position *= _transform;
	 rotate(_transform);
 }

 void Camera::translate(const float _dx, const float _dy, const float _dz) {
	 translate(Vec3(_dx, _dy, _dz));
 }

 void Camera::translate(const Vec3& _delta) {
	 position += _delta;
 }

 Vec3 Camera::unproject(const Vec3& _screenCoords, const float _viewportX, const float _viewportY, const float _viewportWidth, const float _viewportHeight) {	 
	 return UNPROJECT(_screenCoords, getAsMat(), projection, Vec4(_viewportX, _viewportY, _viewportWidth, _viewportHeight));
 }

 Vec3 Camera::unproject(const Vec3& _screenCoords) {
	 return unproject(_screenCoords, 0.f, 0.f, static_cast<float>(M_WIDTH), static_cast<float>(M_HEIGHT));
 }

 Vec3 Camera::project(const Vec3& _worldCoords, const float _viewportX, const float _viewportY, const float _viewportWidth, const float _viewportHeight) {
	 return PROJECT(_worldCoords, getAsMat(), projection, Vec4(_viewportX, _viewportY, _viewportWidth, _viewportHeight));
 }

 Vec3 Camera::project(const Vec3& _worldCoords) {
	 return project(_worldCoords, 0.f, 0.f, static_cast<float>(M_WIDTH), static_cast<float>(M_HEIGHT));
 }

 const Ray* Camera::getPickRay(const float _screenX, const float _screenY, const float _viewportX, const float _viewportY, const float _viewportWidth, const float _viewportHeight) {
	 ray->origin = unproject(Vec3(_screenX, _screenY, 0.f), _viewportX, _viewportY, _viewportWidth, _viewportHeight);
	 ray->direction = NOR(unproject(Vec3(_screenX, _screenY, 1.f), _viewportX, _viewportY, _viewportWidth, _viewportHeight) - ray->origin);
	 return ray;
 }

 const Ray* Camera::getPickRay(const float _screenX, const float _screenY) {
	 return getPickRay(_screenX, _screenY, 0.f, 0.f, static_cast<float>(M_WIDTH), static_cast<float>(M_HEIGHT));
 }

 PerspectiveCamera::PerspectiveCamera() : PerspectiveCamera(67.f, static_cast<float>(M_WIDTH), static_cast<float>(M_HEIGHT)){}

 PerspectiveCamera::PerspectiveCamera(const float _fieldOfViewY, const float _viewportWidth, const float _viewportHeight) :
	 fieldOfView(_fieldOfViewY), Camera(_viewportWidth, _viewportHeight){
 }

 void PerspectiveCamera::update() {
	 update(true);
 }

 void PerspectiveCamera::update(const bool _updateFrustum) {
	 float aspect = viewportWidth / viewportHeight;
	 projection = PERSPECTIVE(fieldOfView, aspect, ABS(nearPlane), ABS(farPlane));
	 view = LOOKAT(position, position + direction, up);
	 combined = projection * view;

	 if (_updateFrustum) frustum->update(combined);
 }

 OrthographicCamera::OrthographicCamera() : OrthographicCamera (static_cast<float>(M_WIDTH), static_cast<float>(M_HEIGHT)){}

 OrthographicCamera::OrthographicCamera(const float _viewportWidth, const float _viewportHeight) : Camera(_viewportWidth, _viewportHeight) {}

 void OrthographicCamera::update() {
	 update(true);
 }

 void OrthographicCamera::update(const bool _updateFrustum) {
	 projection = ORTHO(zoom * -viewportWidth * 0.5f, zoom * (viewportWidth * 0.5f),
		 zoom * -(viewportHeight * 0.5f), zoom * viewportHeight * 0.5f, nearPlane, farPlane);
	 view = LOOKAT(position, position + direction, up);
	 combined = projection * view;

	 if (_updateFrustum) frustum->update(combined);
 }

 void OrthographicCamera::setToOrtho(const bool _yDown) {
	 setToOrtho(_yDown, static_cast<float>(M_WIDTH), static_cast<float>(M_HEIGHT));
 }

 void OrthographicCamera::setToOrtho(const bool _yDown, const float _viewportWidth, const float _viewportHeight) {
	 if (_yDown) {
		 up = Vec3(0.f, -1.f, 0.f);
		 direction = Vec3(0.f, 0.f, 1.f);
	 } else {
		 up = Vec3(0.f, 1.f, 0.f);
		 direction = Vec3(0.f, 0.f, -1.f);
	 }
	 position = Vec3(zoom * _viewportWidth * 0.5f, zoom * _viewportHeight * 0.5f, 0.f);
	 viewportWidth = _viewportWidth;
	 viewportHeight = _viewportHeight;
	 update();
 }

 void OrthographicCamera::rotate(const float _angle) {
	 Camera::rotate(direction, _angle);
 }

 void OrthographicCamera::translate(const float _dx, const float _dy) {
	 Camera::translate(_dx, _dy, 0.f);
 }

 void OrthographicCamera::translate(const Vec2& _vec) {
	 translate(_vec.x, _vec.y);
 }

 AxisWidgetCamera::AxisWidgetCamera(Camera* _camera) : Camera(200.f, 200.f){

	 cam = _camera;

	 nearPlane = 0.1f;
	 farPlane = 50.f;

	 direction = Vec3(0.f, 0.f, -1.f);
	 up = Vec3(0.f, 1.f, 0.f);
	 right = Vec3(1.f, 0.f, 0.f);
	 position = Vec3(0.f, 0.f, 100.f);

	 projection = ORTHO(-viewportWidth * 0.5f, (viewportWidth * 0.5f),
		 -(viewportHeight * 0.5f), viewportHeight * 0.5f, nearPlane, farPlane);
	 view = LOOKAT(position, position + direction, up);
	 combined = projection * view;

	 const std::string vertex =
		 "#version 460 core \n"
		 "layout (location = 0) in vec3 a_Pos;"
		 "layout (location = 1) in vec4 a_Col;"
		 "out vec4 col;"
		 "uniform mat4 transform;"
		 "void main() {"
		 "gl_Position = transform * vec4(a_Pos.xyz, 1.0);"
		 "col = a_Col;"
		 "}";

	 std::string fragment =
		 "#version 460 core \n"
		 "out vec4 FragColor;"
		 "in vec4 col;"
		 "void main(){"
		 "FragColor = col;"
		 "}";

	 shader = new ShaderProgram();
	 shader->loadFromMemory("TransformWidget Shader", "", vertex, "", fragment);
	 camLocation = glGetUniformLocation(shader->getHandle(), "transform");

	 float* vertexBuffer = new float[4 * 6];
	 unsigned int* indexBuffer = new unsigned int[6];

	 float l = 90.f;
	 int k = 0;
	 //x arrow
	 vertexBuffer[k] = 0.f; //x
	 vertexBuffer[++k] = 0.f; //y
	 vertexBuffer[++k] = 0.f; //z
	 vertexBuffer[++k] = M_FloatBits(sf::Color::Red); //c

	 vertexBuffer[++k] = l; //x
	 vertexBuffer[++k] = 0.f; //y
	 vertexBuffer[++k] = 0.f; //z
	 vertexBuffer[++k] = M_FloatBits(sf::Color::Red); //c

		 //y arrow
	 vertexBuffer[++k] = 0.f; //x
	 vertexBuffer[++k] = 0.f; //y
	 vertexBuffer[++k] = 0.f; //z
	 vertexBuffer[++k] = M_FloatBits(sf::Color::Green); //c

	 vertexBuffer[++k] = 0.f; //x
	 vertexBuffer[++k] = l; //y
	 vertexBuffer[++k] = 0.f; //z
	 vertexBuffer[++k] = M_FloatBits(sf::Color::Green); //c

		 //z arrow
	 vertexBuffer[++k] = 0.f; //x
	 vertexBuffer[++k] = 0.f; //y
	 vertexBuffer[++k] = 0.f; //z
	 vertexBuffer[++k] = M_FloatBits(sf::Color::Blue); //c

	 vertexBuffer[++k] = 0.f; //x
	 vertexBuffer[++k] = 0.f; //y
	 vertexBuffer[++k] = l; //z
	 vertexBuffer[++k] = M_FloatBits(sf::Color::Blue); //c

	 k = 0;
	 indexBuffer[k] = 0;
	 indexBuffer[++k] = 1;

	 indexBuffer[++k] = 2;
	 indexBuffer[++k] = 3;

	 indexBuffer[++k] = 4;
	 indexBuffer[++k] = 5;

	 GLuint vbo, index;

	 //create buffer
	 glGenVertexArrays(1, &vao);
	 glGenBuffers(1, &vbo);
	 glGenBuffers(1, &index);

	 glBindVertexArray(vao);

	 //vbo
	 glBindBuffer(GL_ARRAY_BUFFER, vbo);
	 glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, vertexBuffer, GL_STATIC_DRAW);

	 //index
	 glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index);
	 glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, indexBuffer, GL_STATIC_DRAW);

	 glEnableVertexAttribArray(0); //pos(3)
	 glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	 glEnableVertexAttribArray(1); //color1 (1)
	 glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 4 * sizeof(float), (void*)(3 * sizeof(float)));

	 glBindBuffer(GL_ARRAY_BUFFER, 0);
	 glBindVertexArray(0);

	 delete vertexBuffer;
	 delete indexBuffer;
 }

 void AxisWidgetCamera::update() {
	 update(true);
 }

 void AxisWidgetCamera::update(const bool) {
	 if (cam == nullptr) return;

	 glViewport(0, 0, static_cast<GLuint>(viewportWidth), static_cast<GLuint>(viewportHeight));

	// transform->setToRotation(cam->direction, cam->up);

	 shader->bind();
	// glUniformMat4fv(camLocation, 1, false, cam->combined->val); //TODO
	 glBindVertexArray(vao);
	 glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, 0);
	 shader->unbind();

	 glViewport(0, 0, static_cast<GLuint>(cam->viewportWidth), static_cast<GLuint>(cam->viewportHeight));
 }

 ArcballCamera::ArcballCamera() : PerspectiveCamera() {}

 ArcballCamera::ArcballCamera(const float _fieldOfViewY, const float _viewportWidth, const float _viewportHeight) :
	PerspectiveCamera(_fieldOfViewY, _viewportWidth, _viewportHeight) {}
