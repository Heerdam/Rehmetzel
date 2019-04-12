
#include "CameraUtils.hpp"
#include "World.hpp"
#include "InputMultiplexer.hpp"
#include "Utils.hpp"

using namespace Heerbann;

void Viewport::setBounds(int _x, int _y, int _width, int _height) {
	//float fx = 1.f / Main::width();
	//float fy = 1.f / Main::height();
	cam.setSize(sf::Vector2f((float)_width, (float)_height));
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
	cam.setSize(sf::Vector2f((float)width, (float)height));
	cam.setCenter(sf::Vector2f(0.f, 0.f));
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
			cam.move(sf::Vector2f(delta));
			last.x = _x;
			last.y = _y;
			return true;
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
		 rec.setPosition(sf::Vector2f((float)posX, (float)(mh - posY - height)));
		 rec.setSize(sf::Vector2f((float)border, (float)height));
		 _window.draw(rec);
		 //right
		 rec.setPosition(sf::Vector2f((float)(posX + width - border), (float)(mh - posY - height)));
		 rec.setSize(sf::Vector2f((float)border, (float)height));
		 _window.draw(rec);
		 //bottom
		 rec.setPosition(sf::Vector2f((float)posX, (float)(mh - posY - border)));
		 rec.setSize(sf::Vector2f((float)width, (float)border));
		 _window.draw(rec);
		 //top
		 rec.setFillColor(sf::Color::Red);
		 rec.setPosition(sf::Vector2f((float)posX, (float)(mh - posY - height)));
		 rec.setSize(sf::Vector2f((float)width, (float)topBorder));
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

	 world->AABB((b2QueryCallback*)this, sf::Vector2f(p1x * UNRATIO, p1y * UNRATIO), sf::Vector2f(p2x * UNRATIO, p2y * UNRATIO));
	 
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

 Camera::Camera(float _viewportWidth, float _viewportHeight) :viewportWidth(_viewportWidth), viewportHeight(_viewportHeight) {}

 void Camera::lookAt(float _x, float _y, float _z) {
	 sf::Vector3f tmpVec = Main::nor(sf::Vector3f(_x, _y, _z) - position);
	 if (!(Main::almost_equal(tmpVec.x, 0.f) && Main::almost_equal(tmpVec.y, 0.f) && Main::almost_equal(tmpVec.z, 0.f))) {
		 float dot = tmpVec.x * up.x + tmpVec.y * up.y + tmpVec.z * up.z; // up and direction must ALWAYS be orthonormal vectors
		 if (Main::almost_equal(std::abs(dot - 1.f), 0.f))			 
			 up = direction * -1.f; // Collinear
		 else if (Main::almost_equal(std::abs(dot + 1.f), 0.f))			
			 up = direction;  // Collinear opposite
		 direction = tmpVec;
		 normalizeUp();
	 }
 }

 void Camera::lookAt(const sf::Vector3f& _pos) {
	 lookAt(_pos.x, _pos.y, _pos.z);
 }

 void Camera::normalizeUp() {
	 sf::Vector3f tmpVec = Main::nor(Main::crs(direction, up));
	 up = Main::nor(Main::crs(direction, tmpVec));
 }

 void Camera::rotate(float _angle, float _axisX, float _axisY, float _axisZ) {
	 tmpMat->setToRotation(_angle, _axisX, _axisY, _axisZ);
	 direction = direction * tmpMat;
	 up = up * tmpMat;
 }

 void Camera::rotate(const sf::Vector3f& _axis, float _angle) {
	 rotate(_axis.x, _axis.y, _axis.z, _angle);
 }

 void Camera::rotate(Matrix4* _transform) {
	 float* l_mat = _transform->val;
	 direction = sf::Vector3f(direction.x * l_mat[Matrix4::M00] + direction.y * l_mat[Matrix4::M01] + direction.z * l_mat[Matrix4::M02],
		 direction.x * l_mat[Matrix4::M10] + direction.y * l_mat[Matrix4::M11] + direction.z * l_mat[Matrix4::M12],
		 direction.x * l_mat[Matrix4::M20] + direction.y * l_mat[Matrix4::M21] + direction.z * l_mat[Matrix4::M22]);
	 up = sf::Vector3f(up.x * l_mat[Matrix4::M00] + up.y * l_mat[Matrix4::M01] + up.z * l_mat[Matrix4::M02],
		 up.x * l_mat[Matrix4::M10] + up.y * l_mat[Matrix4::M11] + up.z * l_mat[Matrix4::M12],
		 up.x * l_mat[Matrix4::M20] + up.y * l_mat[Matrix4::M21] + up.z * l_mat[Matrix4::M22]);
 }

 void Camera::rotate(Quaternion* _quat) {
	 _quat->transform(direction);
	 _quat->transform(up);
 }

 void Camera::rotateAround(const sf::Vector3f& _point, const sf::Vector3f& _axis, float _angle) {
	 sf::Vector3f tmpVec = _point - position;
	 translate(tmpVec);
	 rotate(_axis, _angle);
	 tmpMat->setToRotation(_angle, _axis.x, _axis.y, _axis.z);
	 tmpVec = tmpVec * tmpMat;
	 translate(-tmpVec.x, -tmpVec.y, -tmpVec.z);
 }

 void Camera::transform(Matrix4* _transform) {
	 position = position * _transform;
	 rotate(_transform);
 }

 void Camera::translate(float _dx, float _dy, float _dz) {
	 translate(sf::Vector3f(_dx, _dy, _dz));
 }

 void Camera::translate(const sf::Vector3f& _delta) {
	 position += _delta;
 }

 sf::Vector3f Camera::unproject(sf::Vector3f& _screenCoords, float _viewportX, float _viewportY, float _viewportWidth, float _viewportHeight) {
	 float x = _screenCoords.x, y = _screenCoords.y;
	 x = x - _viewportX;
	 y = Main::height() - y - 1;
	 y = y - _viewportY;
	 _screenCoords.x = (2 * x) / _viewportWidth - 1;
	 _screenCoords.y = (2 * y) / _viewportHeight - 1;
	 _screenCoords.z = 2 * _screenCoords.z - 1;

	 const float* l_mat = invProjectionView->val;
	 const float l_w = 1.f / (_screenCoords.x * l_mat[Matrix4::M30] + _screenCoords.y * l_mat[Matrix4::M31] + _screenCoords.z * l_mat[Matrix4::M32] + l_mat[Matrix4::M33]);
	 _screenCoords = sf::Vector3f((_screenCoords.x * l_mat[Matrix4::M00] + _screenCoords.y * l_mat[Matrix4::M01] + _screenCoords.z * l_mat[Matrix4::M02] + l_mat[Matrix4::M03]) * l_w, 
		 (x * l_mat[Matrix4::M10] + y * l_mat[Matrix4::M11] + _screenCoords.z * l_mat[Matrix4::M12] + l_mat[Matrix4::M13]) * l_w, 
		 (_screenCoords.x * l_mat[Matrix4::M20] + _screenCoords.y * l_mat[Matrix4::M21] + _screenCoords.z * l_mat[Matrix4::M22] + l_mat[Matrix4::M23]) * l_w);
	 return _screenCoords;
 }

 sf::Vector3f Camera::unproject(sf::Vector3f& _screenCoords) {
	 return unproject(_screenCoords, 0.f, 0.f, Main::width(), Main::height());
 }

 sf::Vector3f Camera::project(sf::Vector3f& _worldCoords, float _viewportX, float _viewportY, float _viewportWidth, float _viewportHeight) {
	 const float* l_mat = combined->val;
	 const float l_w = 1.f / (_worldCoords.x * l_mat[Matrix4::M30] + _worldCoords.y * l_mat[Matrix4::M31] + _worldCoords.z * l_mat[Matrix4::M32] + l_mat[Matrix4::M33]);
	 _worldCoords = sf::Vector3f((_worldCoords.x * l_mat[Matrix4::M00] + _worldCoords.y * l_mat[Matrix4::M01] + _worldCoords.z * l_mat[Matrix4::M02] + l_mat[Matrix4::M03]) * l_w,
		 (_worldCoords.x * l_mat[Matrix4::M10] + _worldCoords.y * l_mat[Matrix4::M11] + _worldCoords.z * l_mat[Matrix4::M12] + l_mat[Matrix4::M13]) * l_w,
		 (_worldCoords.x * l_mat[Matrix4::M20] + _worldCoords.y * l_mat[Matrix4::M21] + _worldCoords.z * l_mat[Matrix4::M22] + l_mat[Matrix4::M23]) * l_w);

	 _worldCoords.x = _viewportWidth * (_worldCoords.x + 1.f) / 2.f + _viewportX;
	 _worldCoords.y = _viewportHeight * (_worldCoords.y + 1.f) / 2.f + _viewportY;
	 _worldCoords.z = (_worldCoords.z + 1.f) / 2.f;
	 return _worldCoords;
 }

 sf::Vector3f Camera::project(sf::Vector3f& _worldCoords) {
	 return project(_worldCoords, 0.f, 0.f, Main::width(), Main::height());
 }

 const Ray* Camera::getPickRay(float _screenX, float _screenY, float _viewportX, float _viewportY, float _viewportWidth, float _viewportHeight) {
	 unproject(ray->origin = sf::Vector3f(_screenX, _screenY, 0), _viewportX, _viewportY, _viewportWidth, _viewportHeight);
	 unproject(ray->direction = sf::Vector3f(_screenX, _screenY, 1), _viewportX, _viewportY, _viewportWidth, _viewportHeight);
	 ray->direction = Main::nor(ray->direction - ray->origin);
	 return ray;
 }

 const Ray* Camera::getPickRay(float _screenX, float _screenY) {
	 return getPickRay(_screenX, _screenY, 0.f, 0.f, Main::width(), Main::height());
 }

 PerspectiveCamera::PerspectiveCamera() : PerspectiveCamera(67, Main::width(), Main::height()){}

 Heerbann::PerspectiveCamera::PerspectiveCamera(float _fieldOfViewY, float _viewportWidth, float _viewportHeight) :
	 fieldOfView(_fieldOfViewY), Camera(_viewportWidth, _viewportHeight){
 }

 void PerspectiveCamera::update() {
	 update(true);
 }

 void PerspectiveCamera::update(bool _updateFrustum) {
	 float aspect = viewportWidth / viewportHeight;
	 projection->setToProjection(std::abs(nearPlane), std::abs(farPlane), fieldOfView, aspect);
	 view->setToLookAt(position, position + direction, up);
	 combined->set(projection);
	 Matrix4::matrix4_mul(combined->val, view->val);

	 if (_updateFrustum) {
		 invProjectionView->set(combined);
		 Matrix4::matrix4_inv(invProjectionView->val);
		 frustum->update(invProjectionView);
	 }
 }

 OrthographicCamera::OrthographicCamera() : OrthographicCamera (Main::width(), Main::height()){}

 OrthographicCamera::OrthographicCamera(float _viewportWidth, float _viewportHeight) : Camera(_viewportWidth, _viewportHeight) {}

 void OrthographicCamera::update() {
	 update(true);
 }

 void OrthographicCamera::update(bool _updateFrustum) {
	 projection->setToOrtho(zoom * -viewportWidth * 0.5f, zoom * (viewportWidth * 0.5f), 
		 zoom * -(viewportHeight * 0.5f), zoom * viewportHeight * 0.5f, nearPlane, farPlane);
	 view->setToLookAt(position, position + direction, up);
	 combined->set(projection);
	 Matrix4::matrix4_mul(combined->val, view->val);

	 if (_updateFrustum) {
		 invProjectionView->set(combined);
		 Matrix4::matrix4_inv(invProjectionView->val);
		 frustum->update(invProjectionView);
	 }
 }

 void OrthographicCamera::setToOrtho(bool _yDown) {
	 setToOrtho(_yDown, Main::width(), Main::height());
 }

 void OrthographicCamera::setToOrtho(bool _yDown, float _viewportWidth, float _viewportHeight) {
	 if (_yDown) {
		 up = sf::Vector3f(0.f, -1.f, 0.f);
		 direction = sf::Vector3f(0.f, 0.f, 1.f);
	 } else {
		 up = sf::Vector3f(0.f, 1.f, 0.f);
		 direction = sf::Vector3f(0.f, 0.f, -1.f);
	 }
	 position = sf::Vector3f(zoom * viewportWidth * 0.5f, zoom * viewportHeight * 0.5f, 0.f);
	 viewportWidth = viewportWidth;
	 viewportHeight = viewportHeight;
	 update();
 }

 void OrthographicCamera::rotate(float _angle) {
	 Camera::rotate(direction, _angle);
 }

 void OrthographicCamera::translate(float _dx, float _dy) {
	 Camera::translate(_dx, _dy, 0.f);
 }

 void OrthographicCamera::translate(sf::Vector2f _vec) {
	 translate(_vec.x, _vec.y);
 }
