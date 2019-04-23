
#include "CameraUtils.hpp"
#include "World.hpp"
#include "InputMultiplexer.hpp"
#include "Utils.hpp"
#include "Math.hpp"

using namespace Heerbann;

void Viewport::setBounds(int _x, int _y, int _width, int _height) {
	//float fx = 1.f / Main::width();
	//float fy = 1.f / Main::height();
	cam.setSize(Vec2((float)_width, (float)_height));
	//cam.setViewport(sf::FloatRect(fx * _x, fy * _y, fx * _width, fy * _height));
}

void Viewport::setSize(int _width, int _height) {
	int maxWidth = Main::width() - posX;
	int maxHeight = Main::height() - posY;
	width = std::clamp(_width, 2 * border, maxWidth);
	height = std::clamp(_height, border + topBorder, maxHeight);

	setBounds(posX, posY, width, height);
};

void Viewport::setPosition(int _x, int _y) {
	posX = std::clamp(_x, 0, (int)Main::width());
	posY = std::clamp(_y, 0, (int)Main::height());
	setSize(width, height);
	setBounds(posX, posY, width, height);
};

Viewport::Viewport(std::string _id, int _prio) {
	cam.setSize(Vec2((float)width, (float)height));
	cam.setCenter(Vec2(0.f, 0.f));
	InputEntry* entry = new InputEntry();
	entry->priority = _prio;

	entry->mouseMoveEvent = [&](int _x, int _y)->bool {
		int bounds = interactive ? inBounds(_x, _y) : 0;
		//std::cout << bounds << std::endl;
		if (mouseRightPressed) { //touch dragged
			sf::Vector2i delta(_x, _y);
			delta -= last;
			delta.x = (int)((float)delta.x * -1 * zoom);
			delta.y = (int)((float)delta.y * zoom);
			cam.move(Vec2(delta));
			last.x = _x;
			last.y = _y;
			return false;
		} else if (resizing || (mouseLeftPressed && bounds > 0)) {
			if (resizing == false) {
				borderRes = bounds;
				resizing = true;
			}
			sf::Vector2i delta(_x, _y);
			delta -= last;
			//std::cout << delta.x << " " << delta.y << std::endl;
			//delta.x = (int)((float)delta.x * - 1 * zoom);
			//delta.y = (int)((float)delta.y * zoom);
			switch (borderRes) {
			case 1://left
			{
				int newPosX = posX + (int)delta.x;

				int newWidth = width - (int)delta.x;
				if (newPosX <= 0)
					newWidth = width + posX;

				int leftC = posX + width;
				if (leftC - newPosX <= 2 * border)
					newPosX = leftC - 2 * border;

				setSize(newWidth, height);
				setPosition(newPosX, posY);
			}
			break;
			case 2: //right
				setSize(width + (int)delta.x, height);
				break;
			case 3: //top (move whole window)
				setPosition(posX + (int)delta.x, posY + (int)delta.y);
				break;
			case 4: //bottom
			{
				int newPosY = posY + (int)delta.y;
				int newHeight = height - (int)delta.y;

				if (newPosY <= 0)
					newHeight = height + posY;

				int topC = posY + height;
				if (topC - newPosY <= border + topBorder) {
					newPosY = topC - (border + topBorder);
					newHeight = border + topBorder;
				}

				setSize(width, newHeight);
				setPosition(posX, newPosY);
			}
			break;
			case 5: //left top corner
				//setPosition(posX + (int)delta.x, posY);
				//setSize(width, height + (int)delta.y);
				break;
			case 6: //left bottom corner
				//setPosition(posX + (int)delta.x, posY);
				//setPosition(posX, posY + (int)delta.y);
				break;
			case 7: //right top corner
			{
				if (exitClick != nullptr) {
					exitClick(_x, _y);
					resizing = false;
					mouseLeftPressed = false;
					borderRes = 0;
				}
			}
			break;
			case 8: //right bottom corner
			{
				int newPosY = posY + (int)delta.y;
				int newHeight = height - (int)delta.y;

				if (newPosY <= 0)
					newHeight = height + posY;

				int topC = posY + height;
				if (topC - newPosY <= border + topBorder) {
					newPosY = topC - (border + topBorder);
					newHeight = border + topBorder;
				}

				setSize(width + (int)delta.x, newHeight);
				setPosition(posX, newPosY);
			}
			break;
			}
			last.x = _x;
			last.y = _y;
			return true;
		}
		//std::cout << last.x << " " << last.y << std::endl;
		last.x = _x;
		last.y = _y;
		return false;
	};

	entry->mouseButtonPressEvent = [&](sf::Mouse::Button _button, int _x, int _y)->bool {
		mouseRightPressed = _button == sf::Mouse::Button::Right ? true : mouseRightPressed;
		mouseLeftPressed = _button == sf::Mouse::Button::Left ? true : mouseLeftPressed;
		last.x = _x;
		last.y = _y;
		return false;
	};

	entry->mouseButtonReleaseEvent = [&](sf::Mouse::Button _button, int _x, int _y)->bool {
		mouseRightPressed = _button == sf::Mouse::Button::Right ? false : mouseRightPressed;

		if (_button == sf::Mouse::Button::Left) {
			resizing = false;
			mouseLeftPressed = false;
			borderRes = 0;
		}

		last.x = _x;
		last.y = _y;
		return false;
	};

	entry->mouseWheelScrollEvent = [&](sf::Mouse::Wheel, float _delta, int, int)->bool {	
		zoom += zoomSpeed * (_delta < 0 ? 1 : -1);
		if(zoom > 0.1f && zoom < 100) cam.zoom(1 + zoomSpeed * (_delta < 0 ? 1 : -1));
		else zoom = std::clamp(zoom, 0.1f, 100.f);		
		return false;
	};

	Main::getInput()->add(_id, entry);
 };

 void Viewport::apply(sf::RenderWindow& _window, float _deltaTime) {
	 
	 glEnable(GL_SCISSOR_TEST);
	 glScissor(posX, posY, width, height);
	 if (clear) {
		 glClearColor(1.f / 255.f * clearColor.r, 1.f / 255.f * clearColor.g, 1.f / 255.f * clearColor.b, 1.f / 255.f * clearColor.a);
		 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	 }
	 _window.setView(cam);
	 
	 glViewport(posX, posY, width, height);

	 if (update != nullptr) update(_window, _deltaTime);
	 if (draw != nullptr) draw(_window, _deltaTime);
	 if (debugDraw) {
		 int mh = Main::height();
		 sf::RectangleShape rec;
		 rec.setFillColor(sf::Color::Blue);
		 //left
		 rec.setPosition(Vec2((float)posX, (float)(mh - posY - height)));
		 rec.setSize(Vec2((float)border, (float)height));
		 _window.draw(rec);
		 //right
		 rec.setPosition(Vec2((float)(posX + width - border), (float)(mh - posY - height)));
		 rec.setSize(Vec2((float)border, (float)height));
		 _window.draw(rec);
		 //bottom
		 rec.setPosition(Vec2((float)posX, (float)(mh - posY - border)));
		 rec.setSize(Vec2((float)width, (float)border));
		 _window.draw(rec);
		 //top
		 rec.setFillColor(sf::Color::Red);
		 rec.setPosition(Vec2((float)posX, (float)(mh - posY - height)));
		 rec.setSize(Vec2((float)width, (float)topBorder));
		 _window.draw(rec);
	 }
	 glDisable(GL_SCISSOR_TEST);
 };

 void Box2dRenderer::draw(float _deltaTime, sf::RenderWindow& _window) {

	 auto cam = Main::getViewport()->cam;
	 auto world = Main::getWorld();

	 auto centre = cam.getCenter();

	 float hw = (Main::width() * 0.5f * 1.1f) * Main::getViewport()->zoom;
	 float hh = (Main::height() * 0.5f * 1.1f) * Main::getViewport()->zoom;

	 float p1x = cam.getCenter().x - hw;
	 float p1y = cam.getCenter().y - hh;

	 float p2x = cam.getCenter().x + hw;
	 float p2y = cam.getCenter().y + hh;

	 world->AABB((b2QueryCallback*)this, Vec2(p1x * UNRATIO, p1y * UNRATIO), Vec2(p2x * UNRATIO, p2y * UNRATIO));
	 
	 //_window.pushGLStates();
	 for (auto o : objects)
		 if (o->draw != nullptr)
			 o->draw(o, _deltaTime, _window);
	 //_window.popGLStates();

	 objects.clear();
 }

 bool Box2dRenderer::ReportFixture(b2Fixture* _fixture) {
	 auto data = (WorldObject*)_fixture->GetBody()->GetUserData();

	 if (data->lastSeen == Main::getFrameId()) return true;
	 if (!data->isLoaded) return true;
	 if (data->isVAO) return true;
	 data->lastSeen = Main::getFrameId();
	 objects.emplace_back(data);

	 return true;
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

 Heerbann::PerspectiveCamera::PerspectiveCamera(const float _fieldOfViewY, const float _viewportWidth, const float _viewportHeight) :
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

	 if (_updateFrustum) {
		 invProjectionView = INV(combined);
		 frustum->update(invProjectionView);
	 }
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

	 if (_updateFrustum) {
		 invProjectionView = INV(combined);
		 frustum->update(invProjectionView);
	 }
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
	 glUniformMat4fv(camLocation, 1, false, cam->combined->val);
	 glBindVertexArray(vao);
	 glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, 0);
	 shader->unbind();

	 glViewport(0, 0, static_cast<GLuint>(cam->viewportWidth), static_cast<GLuint>(cam->viewportHeight));
 }
