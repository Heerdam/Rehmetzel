
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

void BGVAO::set(float* _data, int _vertexCount, int _vertexSize) {
	vertexCount = _vertexCount;
	data = _data;
	vertexSize = _vertexSize;
}


void BGVAO::build(sf::Shader* _shader) {
	
	cameraUniformHandle = glGetUniformLocation(_shader->getNativeHandle(), "transform");

	//create buffer
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * vertexSize, data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	tex[0] = (sf::Texture*)(sf::Image*)(Main::getAssetManager()->getAsset("assets/tex/Forest_soil_diffuse.png")->data);

	tex[1] = (sf::Texture*)(Main::getAssetManager()->getAsset("assets/tex/ForestCliff_basecolor.png")->data);
	tex[2] = (sf::Texture*)(sf::Image*)(Main::getAssetManager()->getAsset("assets/tex/ForestDirt_diffuse.png")->data);

	tex[3] = (sf::Texture*)(sf::Image*)(Main::getAssetManager()->getAsset("assets/tex/ForestGrass_basecolor.png")->data);
	tex[4] = (sf::Texture*)(sf::Image*)(Main::getAssetManager()->getAsset("assets/tex/ForestMoss_basecolor.png")->data);
	tex[5] = (sf::Texture*)(sf::Image*)(Main::getAssetManager()->getAsset("assets/tex/ForestMud_baseColor.png")->data);

	tex[6] = (sf::Texture*)(sf::Image*)(Main::getAssetManager()->getAsset("assets/tex/ForestRoad_diffuse.png")->data);
	tex[7] = (sf::Texture*)(sf::Image*)(Main::getAssetManager()->getAsset("assets/tex/ForestRock_basecolor.png")->data);
	tex[8] = (sf::Texture*)(sf::Image*)(Main::getAssetManager()->getAsset("assets/tex/ForestWetMud_baseColor.png")->data);

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

	for (int i = 0; i < 9; ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, tex[i]->getNativeHandle());
		glUniform1i(texLoc[i], i);
	}

	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, vertexCount); //TODO

	for (int i = 0; i < 9; ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}		

	glBindVertexArray(0);
	glUseProgram(0);
}

void IndexedVAO::set(float* _data, GLuint* _indices, int _vertexCount, int _vertexSize) {
	vertexCount = _vertexCount;
	data = _data;
	indices = _indices;
	vertexSize = _vertexSize;
}

void IndexedVAO::build(sf::Shader* _shader) {
	cameraUniformHandle = glGetUniformLocation(_shader->getNativeHandle(), "transform");
	viewportSizeUniformHandle = glGetUniformLocation(_shader->getNativeHandle(), "viewportSize");
	radiusUniformHandle = glGetUniformLocation(_shader->getNativeHandle(), "radius");

	//create buffer
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &index);

	glBindVertexArray(vao);


	//vbo
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 5, data, GL_STATIC_DRAW);	

	//index
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * (vertexCount / 4 * 6), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	TextureAtlas* atlas = (TextureAtlas*)Main::getAssetManager()->getAsset("assets/trees/trees")->data;
	tex.resize(atlas->tex.size());

	for (int i = 0; i < (int)tex.size(); ++i) {
		tex[i] = atlas->tex[i];
	}

	texLoc.resize(atlas->tex.size());
	for (int i = 0; i < (int)tex.size(); ++i)
		texLoc[i] = glGetUniformLocation(_shader->getNativeHandle(), (std::string("tex[") + std::to_string(i) + std::string("]")).c_str());
}

void IndexedVAO::draw(sf::Shader* _shader) {
	sf::Shader::bind(_shader);
	glUniformMatrix4fv(cameraUniformHandle, 1, false, Main::getViewport()->cam.getTransform().getMatrix());
	glUniform1f(radiusUniformHandle, viewRadius);
	glUniform2f(viewportSizeUniformHandle, (float)Main::width(), (float)Main::height());

	for (int i = 0; i < (int)tex.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, tex[i]->getNativeHandle());
		glUniform1i(texLoc[i], i);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, vertexCount / 4 * 6, GL_UNSIGNED_INT, nullptr);

	glDisable(GL_BLEND);


	for (int i = 0; i < (int)tex.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindVertexArray(0);
	glUseProgram(0);
}

//convert a Box2D (float 0.0f - 1.0f range) color to a SFML color (uint8 0 - 255 range)
sf::Color DebugDraw::B2SFColor(const b2Color& _color, int _alpha = 255) {	
	return sf::Color((sf::Uint8)(_color.r * 255), (sf::Uint8)(_color.g * 255), (sf::Uint8)(_color.b * 255), (sf::Uint8) _alpha);
}

void DebugDraw::DrawAABB(b2AABB* _aabb, const b2Color& _color) {
	sf::ConvexShape polygon;
	polygon.setOutlineColor(B2SFColor(_color));
	polygon.setOutlineThickness(1.f);

	polygon.setPoint(0, sf::Vector2f(_aabb->lowerBound.x*RATIO, _aabb->lowerBound.y*RATIO));
	polygon.setPoint(1, sf::Vector2f(_aabb->upperBound.x*RATIO, _aabb->lowerBound.y*RATIO));
	polygon.setPoint(2, sf::Vector2f(_aabb->upperBound.x*RATIO, _aabb->upperBound.y*RATIO));
	polygon.setPoint(3, sf::Vector2f(_aabb->lowerBound.x*RATIO, _aabb->upperBound.y*RATIO));

	Main::getContext()->draw(polygon);
}

void DebugDraw::DrawString(int _x, int _y, const char* _string) {
	sf::Text fpsText;
	fpsText.setFont(*Main::getDefaultFont());
	fpsText.setCharacterSize(15);
	fpsText.setPosition(sf::Vector2f(_x * RATIO, _y * RATIO));
	fpsText.setString(_string);

	Main::getContext()->draw(fpsText);
}

void DebugDraw::DrawPoint(const b2Vec2& _p, float32 _size, const b2Color& _color) {
	sf::CircleShape shape;
	shape.setPosition(sf::Vector2f(_p.x * RATIO, _p.y * RATIO));
	shape.setRadius(_size * RATIO);
	shape.setFillColor(B2SFColor(_color));

	Main::getContext()->draw(shape);
}

void DebugDraw::DrawTransform(const b2Transform& _xf) {
	float x, y, lineProportion;
	x = _xf.p.x * RATIO;
	y = _xf.p.y * RATIO;
	lineProportion = 0.15f; // 0.15 ~ 10 pixels
	b2Vec2 p1 = _xf.p, p2;

	//red (X axis)
	p2 = p1 + (lineProportion * _xf.q.GetXAxis());
	sf::Vertex redLine[] =
	{
		sf::Vertex(sf::Vector2f(p1.x * RATIO, p1.y * RATIO)),
		sf::Vertex(sf::Vector2f(p2.x * RATIO, p2.y * RATIO))
	};

	//green (Y axis)
	p2 = p1 - (lineProportion * _xf.q.GetYAxis());
	sf::Vertex greenLine[] =
	{
		sf::Vertex(sf::Vector2f(p1.x * RATIO, p1.y * RATIO)),
		sf::Vertex(sf::Vector2f(p2.x  *RATIO, p2.y * RATIO))
	};

	redLine[0].color = sf::Color::Red;
	redLine[1].color = sf::Color::Red;

	greenLine[0].color = sf::Color::Green;
	greenLine[1].color = sf::Color::Green;

	Main::getContext()->draw(redLine, 2, sf::Lines);
	Main::getContext()->draw(greenLine, 2, sf::Lines);
}

void DebugDraw::DrawSegment(const b2Vec2& _p1, const b2Vec2& _p2, const b2Color& _color) {
	auto color = B2SFColor(_color);
	sf::Vertex line[] =
	{
		sf::Vertex(sf::Vector2f(_p1.x * RATIO, _p1.y * RATIO)),
		sf::Vertex(sf::Vector2f(_p2.x  *RATIO, _p2.y * RATIO))
	};

	line[0].color = color;
	line[1].color = color;

	Main::getContext()->draw(line, 2, sf::Lines);
}

void DebugDraw::DrawSolidCircle(const b2Vec2& _center, float32 _radius, const b2Vec2& _axis, const b2Color& _color) {
	sf::CircleShape shape;
	auto color = B2SFColor(_color);
	float radius = _radius * RATIO;
	shape.setOrigin(sf::Vector2f(radius, radius));
	shape.setPosition(sf::Vector2f(_center.x * RATIO, _center.y * RATIO));
	shape.setRadius(radius);
	shape.setOutlineColor(color);
	shape.setOutlineThickness(1.f);
	shape.setFillColor(sf::Color::Transparent);

	// line of the circle wich shows the angle
	b2Vec2 p = _center + (_radius * _axis);
	sf::Vertex line[] =
	{
		sf::Vertex(sf::Vector2f(_center.x * RATIO, _center.y * RATIO)),
		sf::Vertex(sf::Vector2f(p.x * RATIO, p.y * RATIO))
	};

	line[0].color = color;
	line[1].color = color;

	Main::getContext()->draw(shape);
	Main::getContext()->draw(line, 2, sf::Lines);
}

void DebugDraw::DrawCircle(const b2Vec2& _center, float32 _radius, const b2Color& _color) {
	sf::CircleShape shape;
	shape.setPosition(sf::Vector2f(_center.x * RATIO, _center.y * RATIO));
	shape.setRadius(_radius * RATIO);
	shape.setOutlineColor(B2SFColor(_color));

	Main::getContext()->draw(shape);
}

void DebugDraw::DrawSolidPolygon(const b2Vec2* _vertices, int32 _vertexCount, const b2Color& _color) {

	sf::ConvexShape polygon;
	polygon.setPointCount(_vertexCount);
	for (int32 i = 0; i < _vertexCount; i++) {
		b2Vec2 vertex = _vertices[i];
		polygon.setPoint(i, sf::Vector2f(vertex.x * RATIO, vertex.y * RATIO));
	}
	polygon.setOutlineColor(B2SFColor(_color));
	polygon.setOutlineThickness(1.f);
	polygon.setFillColor(sf::Color::Transparent);
	Main::getContext()->draw(polygon);
}

DebugDraw::DebugDraw() {
	SetFlags(e_shapeBit);
}

void DebugDraw::DrawPolygon(const b2Vec2* _vertices, int32 _vertexCount, const b2Color& _color) {
	sf::ConvexShape polygon;
	polygon.setPointCount(_vertexCount);
	for (int32 i = 0; i < _vertexCount; i++) {
		b2Vec2 vertex = _vertices[i];
		polygon.setPoint(i, sf::Vector2f(vertex.x * RATIO, vertex.y * RATIO));
	}
	polygon.setOutlineThickness(1.f);
	polygon.setOutlineColor(B2SFColor(_color));
	polygon.setFillColor(sf::Color::Transparent);
	Main::getContext()->draw(polygon);
}

void DebugDraw::DrawMouseJoint(b2Vec2& p1, b2Vec2& p2, const b2Color &boxColor, const b2Color &lineColor) {
	sf::ConvexShape polygon;
	sf::ConvexShape polygon2;
	float p1x = p1.x * RATIO;
	float p1y = p1.y * RATIO;
	float p2x = p2.x * RATIO;
	float p2y = p2.y * RATIO;
	float size = 4.0f;

	sf::Color boxClr = B2SFColor(boxColor);
	sf::Color lineClr = B2SFColor(lineColor);

	polygon.setOutlineColor(boxClr);
	polygon2.setOutlineColor(boxClr);

	//first green box for the joint
	polygon.setPointCount(4);
	polygon.setPoint(0, sf::Vector2f(p1x - size * 0.5f, p1y - size * 0.5f));
	polygon.setPoint(1, sf::Vector2f(p1x + size * 0.5f, p1y - size * 0.5f));
	polygon.setPoint(2, sf::Vector2f(p1x + size * 0.5f, p1y + size * 0.5f));
	polygon.setPoint(3, sf::Vector2f(p1x - size * 0.5f, p1y + size * 0.5f));

	//second green box for the joint
	polygon2.setPointCount(4);
	polygon2.setPoint(0, sf::Vector2f(p2x - size * 0.5f, p2y - size * 0.5f));
	polygon2.setPoint(1, sf::Vector2f(p2x + size * 0.5f, p2y - size * 0.5f));
	polygon2.setPoint(2, sf::Vector2f(p2x + size * 0.5f, p2y + size * 0.5f));
	polygon2.setPoint(3, sf::Vector2f(p2x - size * 0.5f, p2y + size * 0.5f));

	sf::Vertex line[] =
	{
		sf::Vertex(sf::Vector2f(p1x, p1y)),
		sf::Vertex(sf::Vector2f(p2x, p2y))
	};

	line[0].color = lineClr;
	line[1].color = lineClr;

	Main::getContext()->draw(polygon);
	Main::getContext()->draw(polygon2);

	Main::getContext()->draw(line, 2, sf::Lines);
}

void Heerbann::SpriteBatch::build() {
	std::lock_guard<std::mutex> lock(queueLock);
	int i = 0;
	while (!drawQueue.empty()) {
		sf::Sprite* next = drawQueue.front();
		drawQueue.pop();

		auto pos = next->getGlobalBounds();
		auto uv = next->getTextureRect();

		assert(textures.count(next->getTexture()->getNativeHandle()));
		int index = textures[next->getTexture()->getNativeHandle()];

		int k = 0;
		//bottom right
		data[vertexSize * i] = pos.left + pos.width;
		data[vertexSize * i + ++k] = pos.top;
		data[vertexSize * i + ++k] = (float)index;
		data[vertexSize * i + ++k] = uv.left + uv.width;
		data[vertexSize * i + ++k] = uv.top;
		data[vertexSize * i + ++k] = color.r;
		data[vertexSize * i + ++k] = color.g;
		data[vertexSize * i + ++k] = color.b;
		data[vertexSize * i + ++k] = color.a;

		//top right
		data[vertexSize * i + ++k] = pos.left + pos.width;
		data[vertexSize * i + ++k] = pos.top + pos.height;
		data[vertexSize * i + ++k] = (float)index;
		data[vertexSize * i + ++k] = uv.left + uv.width;
		data[vertexSize * i + ++k] = uv.top + uv.height;
		data[vertexSize * i + ++k] = color.r;
		data[vertexSize * i + ++k] = color.g;
		data[vertexSize * i + ++k] = color.b;
		data[vertexSize * i + ++k] = color.a;

		//top left
		data[vertexSize * i + ++k] = pos.left;
		data[vertexSize * i + ++k] = pos.top + pos.height;
		data[vertexSize * i + ++k] = (float)index;
		data[vertexSize * i + ++k] = uv.left;
		data[vertexSize * i + ++k] = uv.top + uv.height;
		data[vertexSize * i + ++k] = color.r;
		data[vertexSize * i + ++k] = color.g;
		data[vertexSize * i + ++k] = color.b;
		data[vertexSize * i + ++k] = color.a;

		//bottom left
		data[vertexSize * i + ++k] = pos.left;
		data[vertexSize * i + ++k] = pos.top;
		data[vertexSize * i + ++k] = (float)index;
		data[vertexSize * i + ++k] = uv.left;
		data[vertexSize * i + ++k] = uv.top;
		data[vertexSize * i + ++k] = color.r;
		data[vertexSize * i + ++k] = color.g;
		data[vertexSize * i + ++k] = color.b;
		data[vertexSize * i + ++k] = color.a;
	}
}

void Heerbann::SpriteBatch::recompile(int _tex) {
	const std::string vertex =
		"#version 330 core"
		"layout (location = 0) in vec2 aPos;"
		"layout (location = 1) in float ain;"
		"layout (location = 2) in vec2 auv;"
		"layout (location = 3) in vec4 acol;"
		"flat out int index;"
		"out vec2 uv;"
		"out vec4 col;"
		"uniform mat4 transform;"
		"void main() {"
			"gl_Position = transform * vec4(aPos.x, aPos.y, 0.0, 1.0);"
			"index = ain;"
			"uv = auv;"
			"col = acol;"
		"}";

	std::string fragment =
		"#version 330 core"
		"out vec4 FragColor;"
		"flat in int index;"
		"in vec2 uv;"
		"in vec4 col;"
		"uniform sampler2D tex[" + std::string(_tex + "") + "];"
		"void main(){"
			"FragColor = texture(tex[index], uv) + col;"
		"}";

	if (!shader->loadFromMemory(vertex, fragment))
		std::exception("failed to compile shader in SpriteBatch");
}

SpriteBatch::SpriteBatch(TextureAtlas* _atlas) {
	SpriteBatch(_atlas, 1000);
}

SpriteBatch::SpriteBatch(TextureAtlas* _atlas, int _maxSprites) {
	setTextureAtlas(_atlas);
	vertexCount = _maxSprites * 4;
	maxSpritesInBatch = _maxSprites;

	GLuint* idata = new GLuint[_maxSprites * 6];
	data = new float[vertexCount * vertexSize]{ 0 };

	shader = new sf::Shader();

	//create buffer
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &index);

	glBindVertexArray(vao);

	//vbo
	glBindBuffer(GL_ARRAY_BUFFER, vbo);	
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * _maxSprites * 4 * vertexSize, data, GL_DYNAMIC_DRAW);

	//index
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * (_maxSprites * 6), idata, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, vertexSize * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, vertexSize * sizeof(float), (void*)(2 * sizeof(float)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertexSize * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, vertexSize * sizeof(float), (void*)(5 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	texLoc.resize(atlas->tex.size());
	for (unsigned int i = 0; i < (int)atlas->tex.size(); ++i)
		texLoc[i] = glGetUniformLocation(shader->getNativeHandle(), (std::string("tex[") + std::to_string(i) + std::string("]")).c_str());
}

void SpriteBatch::begin() {
	assert(workthread == nullptr && !locked);
	workthread = new std::thread(&SpriteBatch::build, this);
	locked = true;
}

void SpriteBatch::end(sf::Transform& _cam) {
	assert(workthread != nullptr);
	if (workthread->joinable())
		workthread->join();
	delete workthread;
	workthread = nullptr;
	locked = false;
	if (spriteCount == 0) return;
	sf::Shader::bind(shader);
	glUniformMatrix4fv(camLocation, 1, false, _cam.getMatrix());

	for (unsigned int i = 0; i < atlas->tex.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, atlas->tex[i]->getNativeHandle());
		glUniform1i(texLoc[i], i);
	}

	if (isBlending) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, spriteCount * 4 * sizeof(float), data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, spriteCount * 6, GL_UNSIGNED_INT, nullptr);

	if (isBlending) glDisable(GL_BLEND);

	for (unsigned int i = 0; i < atlas->tex.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindVertexArray(0);
	glUseProgram(0);
}

void SpriteBatch::draw(sf::Sprite* _drawable) {
	assert(!locked);
	drawQueue.emplace(_drawable);
}

void Heerbann::SpriteBatch::setTextureAtlas(TextureAtlas* _atlas) {
	textures.clear();
	for (unsigned int i = 0; i < _atlas->tex.size(); ++i) {
		auto tex = _atlas->tex[i];
		textures[tex->getNativeHandle()] = i;
	}
	recompile(_atlas->tex.size());
}

