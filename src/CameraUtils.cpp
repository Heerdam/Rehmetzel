
#include "CameraUtils.hpp"
#include "World.hpp"
#include "InputMultiplexer.hpp"
#include "Utils.hpp"
#include "Math.hpp"
#include "TimeLog.hpp"

using namespace Heerbann;

bool ViewportHandler::checkBounds(const Vec4u& _bounds) {
	if (currentGLBounds == _bounds) return true;
	currentGLBounds[0] = _bounds[0];
	currentGLBounds[1] = _bounds[1];
	currentGLBounds[2] = _bounds[2];
	currentGLBounds[3] = _bounds[3];
	return false;
}

View* ViewportHandler::get(std::string _id) {
	return views[_id];
}

View* ViewportHandler::create(std::string _id, ViewType _type, bool _uniform) {
	View* out = new View(_id, _type, this, _uniform);
	views[_id] = out;
	return out;
}

void ViewportHandler::remove(std::string _id) {
	View* out = views[_id];
	delete out;
	views[_id] = nullptr;
}

void View::setViewportBounds(uint _x, uint _y, uint _width, uint _height) {
	GLBounds[0] = _x;
	GLBounds[1] = _y;
	GLBounds[2] = _width;
	GLBounds[3] = _height;
	camera->viewportWidth = static_cast<float>(_width);
	camera->viewportHeight = static_cast<float>(_height);
}

void View::clear(const sf::Color _color) {
	glClearColor(colF(_color.r), colF(_color.g), colF(_color.b), colF(_color.a));
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void View::drawDebug() {
	std::vector<Vec4> corners(4);
	corners[0] = Vec4(GLBounds.x, GLBounds.y, 0.f, 1.f);
	corners[1] = Vec4(GLBounds.x + GLBounds.z, GLBounds.y, 0.f, 1.f);
	corners[2] = Vec4(GLBounds.x + GLBounds.z, GLBounds.y + GLBounds.w, 0.f, 1.f);
	corners[3] = Vec4(GLBounds.x, GLBounds.y + GLBounds.w, 0.f, 1.f);
	M_Shape->loop(corners, sf::Color::Green);
	M_Shape->draw();
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
			if (buttonPressed) {
				switch (type) {
					case ViewType::pers:
						{
							ArcballCamera* cam = reinterpret_cast<ArcballCamera*>(camera);
							cam->azimuth = std::fmod((cam->azimuth + (_x - lastPos.x) * panXModifier), 360.f);
							cam->height = CLAMP((cam->height + (_y - lastPos.y) * panYModifier), 1.f, 179.f);
							camera->arcball(cam->target, cam->azimuth, cam->height, cam->distance);
							lastPos = Vec2i(_x, _y);
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
						job->cam->translate(LERP((job->dir*job->speed), Vec2(0.f, 0.f), (job->t += DeltaTime)));
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
					cam->distance = CLAMP(cam->distance -= _delta * zoomModifier, zoomBounds[0], zoomBounds[1]);
					camera->arcball(cam->target, cam->azimuth, cam->height, cam->distance);
					camera->normalizeUpYLocked();
					camera->update();
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

View::View(std::string _id, ViewType _type, ViewportHandler* _parent, bool _uniform) : parent(_parent), id(_id), type(_type) {
	switch (type) {
		case ViewType::pers:
		{
			ArcballCamera* cam = new ArcballCamera();
			camera = cam;
			setViewportBounds(0, 0, M_WIDTH, M_HEIGHT);			
			zoomBounds = Vec2(1.f, 100.f);
			cam->distance = 100.f;
			camera->arcball(cam->target, cam->azimuth, cam->height, cam->distance);
			apply();
		}
		break;
		case ViewType::ortho:
		{
		}
		break;
		case ViewType::ortho2d:
		{
			OrthographicCamera* cam = new OrthographicCamera();
			camera = cam;
			setViewportBounds(0, 0, M_WIDTH, M_HEIGHT);			
			zoomBounds = Vec2(0.1f, 100.f);
			cam->setToOrtho(false);
		}
		break;
	}	
	if (_uniform) {
		App::Gdx::printOpenGlErrors("pre uniform aloc: " + _id);

		uniBuffers.resize(2u);
		uniform = _uniform;
		GLuint buffers[2];
		glCreateBuffers(2, buffers);
		for (uint i = 0; i < 2u; ++i) {
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[i]);

			uint bufferSize = sizeof(float) * (16 + 2);
			glBufferStorage(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr,
				GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);

			float* data = reinterpret_cast<float*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, bufferSize,
				GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_RANGE_BIT));

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			uniBuffers[i] = std::make_tuple(buffers[i], data);
		}

		App::Gdx::printOpenGlErrors("post uniform aloc: " + _id);
	}
 }

void View::updateUniforms() {
	bufferIndex = (bufferIndex + 1) % 2;
	float* comb = combined();
	float* data = std::get<1>(uniBuffers[bufferIndex]);
	std::memcpy(data, comb, sizeof(float) * 16);
	data[16] = static_cast<float>(GLBounds.z);
	data[17] = static_cast<float>(GLBounds.w);
}

View::~View() {
	setInteractive(false);
}

 void View::apply() {
	 	 
	 if(!parent->checkBounds(GLBounds))
		 glViewport(GLBounds.x, GLBounds.y, GLBounds.z, GLBounds.w);

	 camera->viewportWidth = static_cast<float>(GLBounds.z);
	 camera->viewportHeight = static_cast<float>(GLBounds.w);

	 camera->update();
	 if (uniform) updateUniforms();
 }

 Camera* View::getCamera() {
	 return camera;
 }

 float* View::combined() {
	 return ToArray(camera->combined);
 }

 GLuint View::getUniformBuffer() {
	 return std::get<0>(uniBuffers[bufferIndex]);
 }

 Camera::Camera(const float _viewportWidth, const float _viewportHeight) :viewportWidth(_viewportWidth), viewportHeight(_viewportHeight),
	 frustum(new Frustum()), ray(new Ray()){
	 direction = Vec4(0.f, 0.f, -1.f, 1.f);
	 up = UVY;
 }

 void Camera::lookAt(const float _x, const float _y, const float _z) {
	 direction = NOR(Vec4(_x, _y, _z, 1.f) - position);
	 normalizeUp();
	 view = LOOKAT(Vec3(position), Vec3(_x, _y, _z), Vec3(up));
 }

 void Camera::lookAt(const Vec4& _pos) {
	 lookAt(_pos.x, _pos.y, _pos.z);
 }

 void Camera::normalizeUp() {
	 right = Vec4(NOR(CRS(Vec3(direction), Vec3(up))), 1.f);
	 up = Vec4(NOR(CRS(Vec3(right), Vec3(direction))), 1.f);
 }

 void Camera::normalizeUpYLocked() {
	 right = Vec4(NOR(CRS(Vec3(UVY), Vec3(direction))), 1.f);
	 up = Vec4(NOR(CRS(Vec3(direction), Vec3(right))), 1.f);
 }

 Quat Camera::getRotation(const Quat& _quat) {	
	 return setFromAxes(direction, right, up);
 }

 Mat4 Camera::getAsMat() {
	 return Mat4();
 }

 void Camera::rotate(const float _axisX, const float _axisY, const float _axisZ, const float _angle) {
	 Mat4 rot = ROTATEMATRIX(Mat4(), _angle, Vec3(_axisX, _axisY, _axisZ));
	 direction = rot * direction;
	 up = rot * up;
	 right = rot * right;
 }

 void Camera::rotate(const Vec4& _axis, const float _angle) {
	 rotate(_axis.x, _axis.y, _axis.z, _angle);
 }

 void Camera::rotate(const Mat4& _transform) {	 
	 direction = _transform * direction;
	 up = _transform * up;
	 right = _transform * right;
 }

 void Camera::rotate(const Quat& _quat) {
	 Mat4 rot = TOMAT4(_quat);
	 direction = rot * direction;
	 up = rot * up;
	 right = rot * right;
 }

 void Camera::rotateAround(const Vec4& _point, const Vec4& _axis, const float _angle) {
	 Vec4 tmpVec = _point - position;
	 translate(tmpVec);
	 rotate(_axis, _angle);
	 Mat4 rot = ROTATEMATRIX(Mat4(), _angle, Vec3(_axis));
	 tmpVec = rot * tmpVec;
	 translate(-tmpVec.x, -tmpVec.y, -tmpVec.z);
 }

 void Camera::arcball(const Vec4& _point, const float _azimuth, const float _altitude, const float _radius) {
		
	 float x = _radius * SIN(TORAD(_altitude)) * COS(TORAD(_azimuth));
	 float y = _radius * SIN(TORAD(_altitude)) * SIN(TORAD(_azimuth));
	 float z = _radius * COS(TORAD(_altitude));

	 position.x = x;
	 position.y = z;
	 position.z = y;

	 //lookAt(_point);
	
	 //update();
 }

 void Camera::transform(const Mat4& _transform) {
	 position = _transform * position;
	 rotate(_transform);
 }

 void Camera::translate(const float _dx, const float _dy, const float _dz) {
	 translate(Vec4(_dx, _dy, _dz, 1.f));
 }

 void Camera::translate(const Vec4& _delta) {
	 position += _delta;
 }

 Vec4 Camera::unproject(const Vec2& _screenCoords, const float _viewportX, const float _viewportY, const float _viewportWidth, const float _viewportHeight) {	 
	 return Vec4(UNPROJECT(Vec3(_screenCoords, 1.f), getAsMat(), projection, Vec4(_viewportX, _viewportY, _viewportWidth, _viewportHeight)), 1.f);
 }

 Vec4 Camera::unproject(const Vec2& _screenCoords) {
	 return unproject(_screenCoords, 0.f, 0.f, static_cast<float>(M_WIDTH), static_cast<float>(M_HEIGHT));
 }

 Vec4 Camera::project(const Vec2& _worldCoords, const float _viewportX, const float _viewportY, const float _viewportWidth, const float _viewportHeight) {
	 return Vec4(PROJECT(Vec3(_worldCoords, 1.f), getAsMat(), projection, Vec4(_viewportX, _viewportY, _viewportWidth, _viewportHeight)), 1.f);
 }

 Vec4 Camera::project(const Vec2& _worldCoords) {
	 return project(_worldCoords, 0.f, 0.f, static_cast<float>(M_WIDTH), static_cast<float>(M_HEIGHT));
 }

 const Ray* Camera::getPickRay(const float _screenX, const float _screenY, const float _viewportX, const float _viewportY, const float _viewportWidth, const float _viewportHeight) {
	 ray->origin = unproject(Vec4(_screenX, _screenY, 0.f, 1.f), _viewportX, _viewportY, _viewportWidth, _viewportHeight);
	 ray->direction = NOR(unproject(Vec4(_screenX, _screenY, 1.f, 1.f), _viewportX, _viewportY, _viewportWidth, _viewportHeight) - ray->origin);
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
	 projection = PERSPECTIVE(TORAD(fieldOfView), aspect, ABS(nearPlane), ABS(farPlane));
	 view = LOOKAT(Vec3(position), Vec3(position + direction), Vec3(up));
	 //std::cout << position.x << " " << position.y << " " << position.z << std::endl;
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
	 view = LOOKAT(Vec3(position), Vec3(position + direction), Vec3(up));
	 combined = projection * view;

	 if (_updateFrustum) frustum->update(combined);
 }

 void OrthographicCamera::setToOrtho(const bool _yDown) {
	 setToOrtho(_yDown, static_cast<float>(M_WIDTH), static_cast<float>(M_HEIGHT));
 }

 void OrthographicCamera::setToOrtho(const bool _yDown, const float _viewportWidth, const float _viewportHeight) {
	 if (_yDown) {
		 up = Vec4(0.f, -1.f, 0.f, 1.f);
		 direction = Vec4(0.f, 0.f, 1.f, 1.f);
	 } else {
		 up = Vec4(0.f, 1.f, 0.f, 1.f);
		 direction = Vec4(0.f, 0.f, -1.f, 1.f);
	 }
	 position = Vec4(zoom * _viewportWidth * 0.5f, zoom * _viewportHeight * 0.5f, 0.f, 1.f);
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

	 direction = Vec4(0.f, 0.f, -1.f, 1.f);
	 up = Vec4(0.f, 1.f, 0.f, 1.f);
	 right = Vec4(1.f, 0.f, 0.f, 1.f);
	 position = Vec4(0.f, 0.f, 100.f, 1.f);

	 projection = ORTHO(-viewportWidth * 0.5f, (viewportWidth * 0.5f),
		 -(viewportHeight * 0.5f), viewportHeight * 0.5f, nearPlane, farPlane);
	 view = LOOKAT(Vec3(position), Vec3(position + direction), Vec3(up));
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

 void ArcballCamera::update() {
	 direction = NOR(target - position);
	 normalizeUpYLocked();
	 //normalizeUp();
	 PerspectiveCamera::update(true);
 }
