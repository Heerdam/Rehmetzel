
#include "Utils.hpp"
#include "Assets.hpp"
#include "CameraUtils.hpp"

using namespace Heerbann;

void BoundingBox2f::set(const sf::Vector2f& _min, const sf::Vector2f& _max) {
	min = sf::Vector2f(_min.x < _max.x ? _min.x : _max.x, _min.y < _max.y ? _min.y : _max.y);
	max = sf::Vector2f(_min.x > _max.x ? _min.x : _max.x, _min.y > _max.y ? _min.y : _max.y);
	cnt = (min + max) *0.5f;
	dim = max - min;
}

void BoundingBox2f::set(const BoundingBox2f& _box) {
	set(_box.min, _box.max);
}

void BoundingBox2f::inf() {
	min = sf::Vector2f(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
	max = sf::Vector2f(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());
	cnt = sf::Vector2f(0, 0);
	dim = sf::Vector2f(0, 0);
}

void BoundingBox2f::ext(const sf::Vector2f& point) {
	set(min = sf::Vector2f(std::min(min.x, point.x), std::min(min.y, point.y)),
		max = sf::Vector2f(std::max(max.x, point.x), std::max(max.y, point.y)));
}

void BoundingBox2f::clr() {
	set(min = sf::Vector2f(0, 0), max = sf::Vector2f(0, 0));
}

bool BoundingBox2f::isValid() {
	return min.x <= max.x && min.y <= max.y;
}

void BoundingBox2f::ext(const BoundingBox2f& _bounds) {
	set(min = sf::Vector2f(std::min(min.x, _bounds.min.x), std::min(min.y, _bounds.min.y)),
		max = sf::Vector2f(std::max(max.x, _bounds.max.x), std::max(max.y, _bounds.max.y)));
}

bool BoundingBox2f::contains(const BoundingBox2f& _b) {
	return !isValid() || (min.x <= _b.min.x && min.y <= _b.min.y && max.x >= _b.max.x && max.y >= _b.max.y);
}

bool BoundingBox2f::intersects(const BoundingBox2f& _b) {
	if (!isValid()) return false;

	// test using SAT (separating axis theorem)
	float lx = abs(cnt.x - _b.cnt.x);
	float sumx = (dim.x / 2.0f) + (_b.dim.x / 2.0f);

	float ly = abs(cnt.y - _b.cnt.y);
	float sumy = (dim.y / 2.0f) + (_b.dim.y / 2.0f);

	return (lx <= sumx && ly <= sumy);
}

bool BoundingBox2f::contains(const sf::Vector2f& _v) {
	return min.x <= _v.x && max.x >= _v.x && min.y <= _v.y && max.y >= _v.y;
}

void BoundingBox2f::ext(float _x, float _y) {
	return set(min = sf::Vector2f(std::min(min.x, _x), std::min(min.y, _y)), max = sf::Vector2f(std::max(max.x, _x), std::max(max.y, _y)));
}

BoundingBox2f::BoundingBox2f() {
	clr();
};

BoundingBox2f::BoundingBox2f(sf::Vector2f _min, sf::Vector2f _max) {
	set(_min, _max);
};

bool almost_equal(float _f1, float _f2) {
	return std::abs(_f1 - _f2) <= std::numeric_limits<float>::epsilon();
}

void VAO::setData(sf::Shader* _shader, float* _data, int _vertexCount) {
	vertexCount = _vertexCount;
}

void VAO::draw(sf::Shader* _shader) {

}

void BGVAO::setData(sf::Shader* _shader, float *data, int _vertexCount) {
	vertexCount = _vertexCount;

	cameraUniformHandle = glGetUniformLocation(_shader->getNativeHandle(), "transform");

	//create buffer
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 3, data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	for (int i = 0; i < 9; ++i)
		tex[i] = new sf::Texture();

	tex[0]->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/tex/Forest_soil_diffuse.png")->data));

	tex[1]->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/tex/ForestCliff_basecolor.png")->data));
	tex[2]->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/tex/ForestDirt_diffuse.png")->data));

	tex[3]->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/tex/ForestGrass_basecolor.png")->data));
	tex[4]->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/tex/ForestMoss_basecolor.png")->data));
	tex[5]->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/tex/ForestMud_baseColor.png")->data));

	tex[6]->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/tex/ForestRoad_diffuse.png")->data));
	tex[7]->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/tex/ForestRock_basecolor.png")->data));
	tex[8]->loadFromImage(*(sf::Image*)(Main::asset_getAsset("assets/tex/ForestWetMud_baseColor.png")->data));

	for (int i = 0; i < 9; ++i)
		texLoc[i] = glGetUniformLocation(_shader->getNativeHandle(), (std::string("tex[") + std::to_string(i) + std::string("]")).c_str());

	//GLenum err;
	//while ((err = glGetError()) != GL_NO_ERROR) {
	//	std::cout << err << std::endl;
	//}
}

void BGVAO::draw(sf::Shader* _shader) {
	sf::Shader::bind(_shader);
	glUniformMatrix4fv(cameraUniformHandle, 1, false, Main::getViewport()->cam.getTransform().getMatrix());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex[0]->getNativeHandle());
	glUniform1i(texLoc[0], 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex[1]->getNativeHandle());
	glUniform1i(texLoc[1], 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex[2]->getNativeHandle());
	glUniform1i(texLoc[2], 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, tex[3]->getNativeHandle());
	glUniform1i(texLoc[3], 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, tex[4]->getNativeHandle());
	glUniform1i(texLoc[4], 4);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, tex[5]->getNativeHandle());
	glUniform1i(texLoc[5], 5);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, tex[6]->getNativeHandle());
	glUniform1i(texLoc[6], 6);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, tex[7]->getNativeHandle());
	glUniform1i(texLoc[7], 7);

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, tex[8]->getNativeHandle());
	glUniform1i(texLoc[8], 8);

	//GLenum err;
	//while ((err = glGetError()) != GL_NO_ERROR) {
	//	std::cout << err << std::endl;
	//}

	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, vertexCount); //TODO

	glBindVertexArray(0);

	glUseProgram(0);
}
