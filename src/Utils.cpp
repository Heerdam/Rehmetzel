
#include "Utils.hpp"
#include "Assets.hpp"
#include "CameraUtils.hpp"
#include "TextUtil.hpp"

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


void BGVAO::build(ShaderProgram* _shader) {

	cameraUniformHandle = glGetUniformLocation(_shader->getHandle(), "transform");

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
		texLoc[i] = glGetUniformLocation(_shader->getHandle(), (std::string("tex[") + std::to_string(i) + std::string("]")).c_str());

	//GLenum err;
	//while ((err = glGetError()) != GL_NO_ERROR) {
	//	std::cout << err << std::endl;
	//}
}

void BGVAO::draw(ShaderProgram* _shader) {
	_shader->bind();
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
	_shader->unbind();
}

void IndexedVAO::set(float* _data, GLuint* _indices, int _vertexCount, int _vertexSize) {
	vertexCount = _vertexCount;
	data = _data;
	indices = _indices;
	vertexSize = _vertexSize;
}

void IndexedVAO::build(ShaderProgram* _shader) {
	cameraUniformHandle = glGetUniformLocation(_shader->getHandle(), "transform");
	viewportSizeUniformHandle = glGetUniformLocation(_shader->getHandle(), "viewportSize");
	radiusUniformHandle = glGetUniformLocation(_shader->getHandle(), "radius");

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
		texLoc[i] = glGetUniformLocation(_shader->getHandle(), (std::string("tex[") + std::to_string(i) + std::string("]")).c_str());
}

void IndexedVAO::draw(ShaderProgram* _shader) {
	_shader->bind();
	glUniformMatrix4fv(cameraUniformHandle, 1, false, Main::getViewport()->cam.getTransform().getMatrix());
	glUniform1f(radiusUniformHandle, viewRadius);
	glUniform2f(viewportSizeUniformHandle, (float)Main::width(), (float)Main::height());

	for (int i = 0; i < (int)tex.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, tex[i]->getNativeHandle());
		glUniform1i(texLoc[i], i);
	}

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, vertexCount / 4 * 6, GL_UNSIGNED_INT, nullptr);

	//glDisable(GL_BLEND);


	for (int i = 0; i < (int)tex.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindVertexArray(0);
	_shader->unbind();
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

void SpriteBatch::recompile(int _tex) {
	const std::string vertex =
		"#version 460 core \n"
		"layout (location = 0) in vec2 a_Pos;"
		"layout (location = 1) in float a_index;"
		"layout (location = 2) in float a_typ;"
		"layout (location = 3) in vec2 a_uv;"
		"layout (location = 4) in vec4 a_col1;"
		"layout (location = 5) in vec4 a_col2;"
		"flat out int index;"
		"flat out int type;"
		"out vec2 uv;"
		"out vec4 col1;"
		"out vec4 col2;"
		"uniform mat4 transform;"
		"uniform vec2 widgetPos;"
		"void main() {"
		"vec2 iPos = a_Pos;"
		"if(" + std::to_string(static_cast<int>(TYP_FONT_STATIC)) + "== a_typ){"
		"iPos = iPos + widgetPos;"
		"}"
		"gl_Position = transform * vec4(iPos.xy, 0.0, 1.0);"
		"index = int(a_index);"
		"type = int(a_typ);"
		"uv = a_uv;"
		"col1 = a_col1;"
		"col2 = a_col2;"		
		"}";

	std::string fragment =
		"#version 460 core \n"
		"out vec4 FragColor;"
		"flat in int index;"
		"flat in int type;"
		"in vec2 uv;"
		"in vec4 col1;"
		"in vec4 col2;"
		"uniform sampler2D tex[" + std::to_string(_tex) + "];"
		"void main(){"
		"if(" + std::to_string(static_cast<int>(TYP_SPRITE)) + "== type){"
		"FragColor = texture(tex[index], uv) + col1;"
		"} else if(" + std::to_string(static_cast<int>(TYP_FONT)) + "== type || " + std::to_string(static_cast<int>(TYP_FONT_STATIC)) + "== type){"
		"FragColor = vec4(col1.xyz, texture(tex[index], uv).a);"
		"} else {"
		"FragColor = col1;"
		"}"
		"}";

	
	shader->loadFromMemory("Spritebatch Shader", "", vertex, "", fragment);
	camLocation = glGetUniformLocation(shader->getHandle(), "transform");
	widgetPositionLocation = glGetUniformLocation(shader->getHandle(), "widgetPos");

	texLoc.resize(TEXTURECOUNT);
	for (int i = 0; i < TEXTURECOUNT; ++i)
		texLoc[i] = glGetUniformLocation(shader->getHandle(), (std::string("tex[") + std::to_string(i) + std::string("]")).c_str());

}

void SpriteBatch::compressDrawJobs(std::vector<DrawJob*>& _jobs) { //TODO nicht debugged!!

	for (auto j : _jobs)
		renderQueue.push(j);
	_jobs.clear();
	return;
	//TODO
	/*/
	for (unsigned int i = 0; i < _jobs.size(); ++i) {

		auto start = _jobs[i];

		unsigned int k = i;
		while (start->type != Type::static_font && k + 1 < _jobs.size() && start->type == _jobs[i + 1]->type) {
			switch (start->type) {
				case Type::sprite:
				{
					auto next = _jobs[i + 1];
					start->count += next->count;
					delete next;
				}							
			}
		}

		renderQueue.push(start);
		i = k;

		if (i == _jobs.size() - 1) {			
			break;
		}

	}
	_jobs.clear();
	*/
}

SpriteBatch::SpriteBatch(int _maxTex, int _maxSprites) {
	vertexCount = _maxSprites * 4;
	maxSpritesInBatch = _maxSprites;
	
	data = new float[vertexCount * VERTEXSIZE]{ 0 };

	shader = new ShaderProgram();
	recompile(_maxTex);

	workthread[0] = new std::thread(&SpriteBatch::buildData, this, 0);
	workthread[1] = new std::thread(&SpriteBatch::buildData, this, 1);
	workthread[2] = new std::thread(&SpriteBatch::buildData, this, 2);
	workthread[3] = new std::thread(&SpriteBatch::buildData, this, 3);

	//create buffer
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &index);

	glBindVertexArray(vao);

	//vbo
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * _maxSprites * 4 * VERTEXSIZE, data, GL_DYNAMIC_DRAW);

	//index
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * (_maxSprites * 6), Main::getIndexBuffer(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); //pos(2)
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, VERTEXSIZE * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1); //texture index(1)
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, VERTEXSIZE * sizeof(float), (void*)(2 * sizeof(float)));

	glEnableVertexAttribArray(2); //type(1)
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, VERTEXSIZE * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(3); //uv (2)
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, VERTEXSIZE * sizeof(float), (void*)(4 * sizeof(float)));

	glEnableVertexAttribArray(4); //color1 (1)
	glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, VERTEXSIZE * sizeof(float), (void*)(6 * sizeof(float)));

	glEnableVertexAttribArray(5); //color2 (1)
	glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE, VERTEXSIZE * sizeof(float), (void*)(7 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

Heerbann::SpriteBatch::~SpriteBatch() {
	terminate = true;
	for (int i = 0; i < 4; ++i) {
		if (workthread[i]->joinable())
			workthread[i]->join();
	}
}

void SpriteBatch::buildData(int _index) {

	static std::mutex renderM;

	const std::vector<Item*>& cache = workCache[_index];

	while (true) {

		using namespace std::chrono_literals;

		while (cache.empty() || threadStatus[_index]) {
			std::this_thread::sleep_for(0.1ms);
			if (terminate) return;
		}

		auto _begin = cache.begin();
		auto _end = cache.end();

		int sprites = 0;
		int _offset = _index * 250;
		for (; _begin != _end; ++_begin) {
			Item* next = *_begin;

			switch (next->type) {
			case Type::sprite:
			{
				sf::Sprite* sprite = reinterpret_cast<sf::Sprite*>(next->data);
				auto pos = sprite->getGlobalBounds();
				auto uv = sprite->getTextureRect();

				assert(textures.count(sprite->getTexture()->getNativeHandle()));
				int index = textures[sprite->getTexture()->getNativeHandle()];

				float col = Main::toFloatBits(color);

				float fracW = 1.f / sprite->getTexture()->getSize().x;
				float fracH = 1.f / sprite->getTexture()->getSize().y;

				int k = 0;
				//bottom right
				data[4 * VERTEXSIZE * (_offset + sprites)] = pos.left + pos.width;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = pos.top;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = static_cast<float>(index);
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = TYP_SPRITE;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = static_cast<float>(uv.left + uv.width) * fracW;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = static_cast<float>(uv.top) * fracH;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = col;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = 0.f;

				//top right
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = pos.left + pos.width;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = pos.top + pos.height;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = static_cast<float>(index);
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = TYP_SPRITE;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = static_cast<float>(uv.left + uv.width) * fracW;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = static_cast<float>(uv.top + uv.height) * fracH;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = col;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = 0.f;

				//top left
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = pos.left;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = pos.top + pos.height;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = static_cast<float>(index);
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = TYP_SPRITE;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = static_cast<float>(uv.left) * fracW;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = static_cast<float>(uv.top + uv.height) * fracH;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = col;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = 0.f;

				//bottom left
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = pos.left;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = pos.top;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = static_cast<float>(index);
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = TYP_SPRITE;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = static_cast<float>(uv.left) * fracW;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = static_cast<float>(uv.top) * fracH;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = col;
				data[4 * VERTEXSIZE * (_offset + sprites) + ++k] = 0.f;
				++sprites;
				++spriteCount;

				DrawJob* job = new DrawJob();
				job->count = k;
				job->offset = _offset * 4 * VERTEXSIZE;
				{
					std::lock_guard<std::mutex> lock(renderM);
					renderCache.push_back(job);
				}

			}
			break;
			case Type::font:
			{
				Text::TextBlock* font = reinterpret_cast<Text::TextBlock*>(next->data);
				int size = 0;
				float* fontData = font->draw(size);
				std::memcpy(data + 4 * VERTEXSIZE * spriteCount, fontData, size * VERTEXSIZE * 4 * sizeof(float));
				spriteCount += size;

				DrawJob* job = new DrawJob();
				job->count = size;
				job->offset = _offset * 4 * VERTEXSIZE;
				{
					std::lock_guard<std::mutex> lock(renderM);
					renderCache.push_back(job);
				}
			}
			break;
			case Type::static_font:
			{
				Text::StaticTextBlock* block = reinterpret_cast<Text::StaticTextBlock*>(next->data);

				DrawJob* job = new DrawJob();
				job->type = Type::static_font;
				job->block = block;
				{
					std::lock_guard<std::mutex> lock(renderM);
					renderCache.push_back(job);
				}
			}
			break;
			}
			delete next;
		}
		threadStatus[_index] = true;
	}
}

void SpriteBatch::build() {
	spriteCount = 0;
	locked = true;
	for (int i = 0; i < 4; ++i)
		threadStatus[i] = workCache[i].empty();
}

void SpriteBatch::drawToScreen(const sf::Transform& _cam) {

	using namespace std::chrono_literals;

	while (!(threadStatus[0] && threadStatus[1] && threadStatus[2] && threadStatus[3])) {
		std::this_thread::sleep_for(0.1ms);
	}

	for (int i = 0; i < 4; ++i)
		workCache[i].clear();

	compressDrawJobs(renderCache);

	locked = false;
	if (renderQueue.empty()) return;
	//sf::Shader::bind(shader);
	shader->bind();
	glUniformMatrix4fv(camLocation, 1, false, _cam.getMatrix());

	for (unsigned int i = 0; i < texCache.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texCache[i]->getNativeHandle());
		glUniform1i(texLoc[i], i);
	}

	if (isDirty) { //TODO spritecount gugus
		isDirty = false;
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, spriteCount * 4 * VERTEXSIZE * sizeof(float), data);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	bool isDrawingText = false;
	while (!renderQueue.empty()) {
		DrawJob* job = renderQueue.front();
		renderQueue.pop();
	
		const auto fontCache = Main::getFontCache();

		switch (job->type) {
			case Type::sprite:
				if (isDrawingText) {
					isDrawingText = false;
					fontCache->end();
				}
				glBindVertexArray(vao);
				glDrawElements(GL_TRIANGLES, job->count * 6, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * job->offset * 6));
			break;
			case Type::static_font:			
				if (!isDrawingText) {
					isDrawingText = true;
					fontCache->begin(job->block->vao);
				}				
				fontCache->drawStaticText(shader, widgetPositionLocation, job->block);
			break;
		}
		delete job;
	}

	for (unsigned int i = 0; i < texCache.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindVertexArray(0);
	shader->unbind();
}

void SpriteBatch::draw(Item* _item) {
	if (workCache[0].size() < 250u)
		workCache[0].emplace_back(_item);
	else if (workCache[1].size() < 250u)
		workCache[0].emplace_back(_item);
	else if (workCache[2].size() < 250u)
		workCache[0].emplace_back(_item);
	else if (workCache[3].size() < 250u)
		workCache[0].emplace_back(_item); 
	else std::cout << "Sprite Limit reached!!" << std::endl;
}

void SpriteBatch::draw(sf::Sprite* _drawable) {
	assert(!locked);
	draw(new Item(Type::sprite, _drawable));
}

void SpriteBatch::draw(Text::TextBlock* _drawable) {
	assert(!locked);
	draw(new Item(Type::font, _drawable));
}

void SpriteBatch::draw(Text::StaticTextBlock* _drawable) {
	assert(!locked);
	draw(new Item(Type::static_font, _drawable));
}

void SpriteBatch::addTexture(TextureAtlas* _atlas) {
	assert(!locked && texCache.size() + _atlas->tex.size() <= texLoc.size());
	for (unsigned int i = 0; i < _atlas->tex.size(); ++i) {
		auto tex = _atlas->tex[i];
		textures[tex->getNativeHandle()] = i;
		texCache.emplace_back(tex);
	}
}

void SpriteBatch::addTexture(const sf::Texture* _tex) {
	assert(!locked && texCache.size() + 1 <= texLoc.size());
	texCache.emplace_back(_tex);
	textures[_tex->getNativeHandle()] = texCache.size() - 1;
}

void SpriteBatch::addTexture(sf::Font* _font) {
	addTexture(&_font->getTexture(SMALLFONTSIZE));
	addTexture(&_font->getTexture(MEDIUMFONTSIZE));
	addTexture(&_font->getTexture(BIGFONTSIZE));
}

void SpriteBatch::addTexture(sf::Sprite* _sprite) {
	addTexture(_sprite->getTexture());
}

void ShaderProgram::print(std::string _id, ShaderProgram::Status _compComp, ShaderProgram::Status _compVert, 
	ShaderProgram::Status _compGeom, ShaderProgram::Status _compFrag, ShaderProgram::Status _link, std::string _errorLog) {
	if (!printDebug) return;
	std::cout << "   Shader: " << _id << std::endl;
	std::cout << "Compiling: " 
		<< (_compComp == Status::failed ? " X |" : _compComp == Status::success ? " S |" : " - |")
		<< (_compVert == Status::failed ? " X |" : _compVert == Status::success ? " S |" : " - |")
		<< (_compGeom == Status::failed ? " X |" : _compGeom == Status::success ? " S |" : " - |")
		<< (_compFrag == Status::failed ? " X |" : _compFrag == Status::success ? " S |" : " - |")
		<< std::endl;
	std::cout << "  Linking: " << (_link == Status::failed ? "Failed!" : _link == Status::success ? "Success!" : " - ") << std::endl;

	if (_errorLog.empty())
		std::cout << std::endl;
	else std::cout << std::endl << _errorLog << std::endl;
	Main::printOpenGlErrors(_id);
	std::cout << std::endl;
}

bool ShaderProgram::compile(const std::string& _id, const char* _compute, const char* _vertex, const char* _geom, const char* _frag) {
	Status compStatus = Status::missing;
	Status vertStatus = Status::missing;
	Status geomStatus = Status::missing;
	Status fragStatus = Status::missing;
	Status linkStatus = Status::missing;

	//Compile Compute
	if (_compute != nullptr) {
		compute = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(compute, 1, &_compute, nullptr);
		glCompileShader(compute);
		GLint isCompiled = 0;
		glGetShaderiv(compute, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(compute, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> errorLog(maxLength);
			glGetShaderInfoLog(compute, maxLength, &maxLength, &errorLog[0]);
			glDeleteShader(compute);
			compStatus = Status::failed;
			print(_id, compStatus, vertStatus, geomStatus, fragStatus, linkStatus, std::string(errorLog.begin(), errorLog.end()));
			return false;
		} else compStatus = Status::success;
	} 

	//Compile Vertex
	if (_vertex != nullptr) {
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &_vertex, nullptr);
		glCompileShader(vertex);
		GLint isCompiled = 0;
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> errorLog(maxLength);
			glGetShaderInfoLog(vertex, maxLength, &maxLength, &errorLog[0]);
			glDeleteShader(vertex);
			vertStatus = Status::failed;
			print(_id, compStatus, vertStatus, geomStatus, fragStatus, linkStatus, std::string(errorLog.begin(), errorLog.end()));
			return false;
		} else vertStatus = Status::success;
	}

	//Compile Geom
	if (_geom != nullptr) {
		geom = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geom, 1, &_geom, nullptr);
		glCompileShader(geom);
		GLint isCompiled = 0;
		glGetShaderiv(geom, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(geom, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> errorLog(maxLength);
			glGetShaderInfoLog(geom, maxLength, &maxLength, &errorLog[0]);
			glDeleteShader(geom);
			geomStatus = Status::failed;
			print(_id, compStatus, vertStatus, geomStatus, fragStatus, linkStatus, std::string(errorLog.begin(), errorLog.end()));
			return false;
		} else geomStatus = Status::success;
	}

	//Compile Frag
	if (_frag != nullptr) {
		frag = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(frag, 1, &_frag, nullptr);
		glCompileShader(frag);
		GLint isCompiled = 0;
		glGetShaderiv(frag, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(frag, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> errorLog(maxLength);
			glGetShaderInfoLog(frag, maxLength, &maxLength, &errorLog[0]);
			glDeleteShader(frag);
			fragStatus = Status::failed;
			print(_id, compStatus, vertStatus, geomStatus, fragStatus, linkStatus, std::string(errorLog.begin(), errorLog.end()));
			return false;
		} else fragStatus = Status::success;
	}
	
	//Link
	program = glCreateProgram();
	if (_compute != nullptr) glAttachShader(program, compute);
	if (_vertex != nullptr) glAttachShader(program, vertex);
	if (_geom != nullptr) glAttachShader(program, geom);
	if (_frag != nullptr) glAttachShader(program, frag);

	glLinkProgram(program);

	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE) {
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
		std::vector<GLchar> errorLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &errorLog[0]);
		glDeleteProgram(program);
		glDeleteShader(compute);
		glDeleteShader(vertex);
		glDeleteShader(geom);
		glDeleteShader(frag);
		linkStatus = Status::failed;
		print(_id, compStatus, vertStatus, geomStatus, fragStatus, linkStatus, std::string(errorLog.begin(), errorLog.end()));
		return false;
	} else linkStatus = Status::success;
	
	if (_compute != nullptr)glDetachShader(program, compute);
	if (_vertex != nullptr)glDetachShader(program, vertex);
	if (_geom != nullptr)glDetachShader(program, geom);
	if (_frag != nullptr)glDetachShader(program, frag);

	print(_id, compStatus, vertStatus, geomStatus, fragStatus, linkStatus, "");
	unbind();
	return true;
}

GLuint ShaderProgram::getHandle() {
	return program;
}

bool ShaderProgram::loadFromMemory(const std::string& _id, const std::string& _compute, const std::string& _vertex, const std::string& _geom, const std::string& _frag) {
	return compile(_id, _compute.empty() ? nullptr : _compute.c_str(), _vertex.empty() ? nullptr : _vertex.c_str(), _geom.empty() ? nullptr : _geom.c_str(), _frag.empty() ? nullptr : _frag.c_str());
}

void ShaderProgram::bind() {
	glUseProgram(getHandle());
}

void ShaderProgram::unbind() {
	glUseProgram(0);
}

void Matrix4::matrix4_mul(float * mata, float * matb){
	float tmp[16];
	tmp[M00] = mata[M00] * matb[M00] + mata[M01] * matb[M10] + mata[M02] * matb[M20] + mata[M03] * matb[M30];
	tmp[M01] = mata[M00] * matb[M01] + mata[M01] * matb[M11] + mata[M02] * matb[M21] + mata[M03] * matb[M31];
	tmp[M02] = mata[M00] * matb[M02] + mata[M01] * matb[M12] + mata[M02] * matb[M22] + mata[M03] * matb[M32];
	tmp[M03] = mata[M00] * matb[M03] + mata[M01] * matb[M13] + mata[M02] * matb[M23] + mata[M03] * matb[M33];
	tmp[M10] = mata[M10] * matb[M00] + mata[M11] * matb[M10] + mata[M12] * matb[M20] + mata[M13] * matb[M30];
	tmp[M11] = mata[M10] * matb[M01] + mata[M11] * matb[M11] + mata[M12] * matb[M21] + mata[M13] * matb[M31];
	tmp[M12] = mata[M10] * matb[M02] + mata[M11] * matb[M12] + mata[M12] * matb[M22] + mata[M13] * matb[M32];
	tmp[M13] = mata[M10] * matb[M03] + mata[M11] * matb[M13] + mata[M12] * matb[M23] + mata[M13] * matb[M33];
	tmp[M20] = mata[M20] * matb[M00] + mata[M21] * matb[M10] + mata[M22] * matb[M20] + mata[M23] * matb[M30];
	tmp[M21] = mata[M20] * matb[M01] + mata[M21] * matb[M11] + mata[M22] * matb[M21] + mata[M23] * matb[M31];
	tmp[M22] = mata[M20] * matb[M02] + mata[M21] * matb[M12] + mata[M22] * matb[M22] + mata[M23] * matb[M32];
	tmp[M23] = mata[M20] * matb[M03] + mata[M21] * matb[M13] + mata[M22] * matb[M23] + mata[M23] * matb[M33];
	tmp[M30] = mata[M30] * matb[M00] + mata[M31] * matb[M10] + mata[M32] * matb[M20] + mata[M33] * matb[M30];
	tmp[M31] = mata[M30] * matb[M01] + mata[M31] * matb[M11] + mata[M32] * matb[M21] + mata[M33] * matb[M31];
	tmp[M32] = mata[M30] * matb[M02] + mata[M31] * matb[M12] + mata[M32] * matb[M22] + mata[M33] * matb[M32];
	tmp[M33] = mata[M30] * matb[M03] + mata[M31] * matb[M13] + mata[M32] * matb[M23] + mata[M33] * matb[M33];
	memcpy(mata, tmp, sizeof(float) * 16);
}

Matrix4::Matrix4() {
	val[M00] = 1.f;
	val[M11] = 1.f;
	val[M22] = 1.f;
	val[M33] = 1.f;
}

Matrix4::Matrix4(Matrix4* _mat) {
	operator=(_mat);
}

Matrix4::Matrix4(Quaternion* _quat) {
	operator=(_quat);
}

Matrix4::Matrix4(const sf::Vector3f& _pos, Quaternion* _quat, const sf::Vector3f& _scale) {
	set(_pos.x, _pos.y, _pos.z, _quat->x, _quat->y, _quat->z, _quat->w, _scale.x, _scale.y, _scale.z);
}

Matrix4::Matrix4(const aiVector3D& _pos, const aiQuaternion& _quat, const aiVector3D& _scale) {
	set(_pos.x, _pos.y, _pos.z, _quat.x, _quat.y, _quat.z, _quat.w, _scale.x, _scale.y, _scale.z);
}

Matrix4* Matrix4::operator=(Matrix4* _mat) {
	std::memcpy(val, _mat->val, sizeof(float) * 16);
	return this;
}

Matrix4* Matrix4::operator=(Quaternion* _quat) {
	const float xs = _quat->x * 2.f, ys = _quat->y * 2.f, zs = _quat->z * 2.f;
	const float wx = _quat->w * xs, wy = _quat->w * ys, wz = _quat->w * zs;
	const float xx = _quat->x * xs, xy = _quat->x * ys, xz = _quat->x * zs;
	const float yy = _quat->y * ys, yz = _quat->y * zs, zz = _quat->z * zs;

	val[M00] = (1.0f - (yy + zz));
	val[M01] = (xy - wz);
	val[M02] = (xz + wy);
	val[M03] = 0.f;

	val[M10] = (xy + wz);
	val[M11] = (1.0f - (xx + zz));
	val[M12] = (yz - wx);
	val[M13] = 0.f;

	val[M20] = (xz - wy);
	val[M21] = (yz + wx);
	val[M22] = (1.0f - (xx + yy));
	val[M23] = 0.f;

	val[M30] = 0.f;
	val[M31] = 0.f;
	val[M32] = 0.f;
	val[M33] = 1.0f;
	return this;
}

Matrix4* Heerbann::Matrix4::operator*(Matrix4* _mat) {
	matrix4_mul(val, _mat->val);
	return this;
}

Matrix4* Matrix4::set(const sf::Vector3f& _pos, Quaternion* _quat, const sf::Vector3f& _scale) {
	return set(_pos.x, _pos.y, _pos.z, _quat->x, _quat->y, _quat->z, _quat->w, _scale.x, _scale.y, _scale.z);
}

Matrix4* Matrix4::set(float _translationX, float _translationY, float _translationZ, float _quaternionX, float _quaternionY,
	float _quaternionZ, float _quaternionW, float _scaleX, float _scaleY, float _scaleZ) {
	const float xs = _quaternionX * 2.f, ys = _quaternionY * 2.f, zs = _quaternionZ * 2.f;
	const float wx = _quaternionW * xs, wy = _quaternionW * ys, wz = _quaternionW * zs;
	const float xx = _quaternionX * xs, xy = _quaternionX * ys, xz = _quaternionX * zs;
	const float yy = _quaternionY * ys, yz = _quaternionY * zs, zz = _quaternionZ * zs;

	val[M00] = _scaleX * (1.0f - (yy + zz));
	val[M01] = _scaleY * (xy - wz);
	val[M02] = _scaleZ * (xz + wy);
	val[M03] = _translationX;

	val[M10] = _scaleX * (xy + wz);
	val[M11] = _scaleY * (1.0f - (xx + zz));
	val[M12] = _scaleZ * (yz - wx);
	val[M13] = _translationY;

	val[M20] = _scaleX * (xz - wy);
	val[M21] = _scaleY * (yz + wx);
	val[M22] = _scaleZ * (1.0f - (xx + yy));
	val[M23] = _translationZ;

	val[M30] = 0.f;
	val[M31] = 0.f;
	val[M32] = 0.f;
	val[M33] = 1.0f;
	return this;
}

Matrix4* Matrix4::set(float* _val) {
	std::memcpy(val, _val, 16 * sizeof(float));
	return this;
}

void Matrix4::setToTranslation(const sf::Vector3f& _vec) {
	setToTranslation(_vec.x, _vec.y, _vec.z);
}

void Matrix4::setToTranslation(float _x, float _y, float _z) {
	idt();
	val[M03] = _x;
	val[M13] = _y;
	val[M23] = _z;
}

float Matrix4::operator[](int _index) {
	assert(_index >= 0 && _index < 16);
	return val[_index];
}

void Matrix4::tra() {
	tmp[M00] = val[M00];
	tmp[M01] = val[M10];
	tmp[M02] = val[M20];
	tmp[M03] = val[M30];
	tmp[M10] = val[M01];
	tmp[M11] = val[M11];
	tmp[M12] = val[M21];
	tmp[M13] = val[M31];
	tmp[M20] = val[M02];
	tmp[M21] = val[M12];
	tmp[M22] = val[M22];
	tmp[M23] = val[M32];
	tmp[M30] = val[M03];
	tmp[M31] = val[M13];
	tmp[M32] = val[M23];
	tmp[M33] = val[M33];
	set(tmp);
}

void Matrix4::idt() {
	val[M00] = 1.f;
	val[M01] = 0.f;
	val[M02] = 0.f;
	val[M03] = 0.f;
	val[M10] = 0.f;
	val[M11] = 1.f;
	val[M12] = 0.f;
	val[M13] = 0.f;
	val[M20] = 0.f;
	val[M21] = 0.f;
	val[M22] = 1.f;
	val[M23] = 0.f;
	val[M30] = 0.f;
	val[M31] = 0.f;
	val[M32] = 0.f;
	val[M33] = 1.f;
}

float Matrix4::det() {
	return val[M30] * val[M21] * val[M12] * val[M03] - val[M20] * val[M31] * val[M12] * val[M03] - val[M30] * val[M11]
		* val[M22] * val[M03] + val[M10] * val[M31] * val[M22] * val[M03] + val[M20] * val[M11] * val[M32] * val[M03] - val[M10]
		* val[M21] * val[M32] * val[M03] - val[M30] * val[M21] * val[M02] * val[M13] + val[M20] * val[M31] * val[M02] * val[M13]
		+ val[M30] * val[M01] * val[M22] * val[M13] - val[M00] * val[M31] * val[M22] * val[M13] - val[M20] * val[M01] * val[M32]
		* val[M13] + val[M00] * val[M21] * val[M32] * val[M13] + val[M30] * val[M11] * val[M02] * val[M23] - val[M10] * val[M31]
		* val[M02] * val[M23] - val[M30] * val[M01] * val[M12] * val[M23] + val[M00] * val[M31] * val[M12] * val[M23] + val[M10]
		* val[M01] * val[M32] * val[M23] - val[M00] * val[M11] * val[M32] * val[M23] - val[M20] * val[M11] * val[M02] * val[M33]
		+ val[M10] * val[M21] * val[M02] * val[M33] + val[M20] * val[M01] * val[M12] * val[M33] - val[M00] * val[M21] * val[M12]
		* val[M33] - val[M10] * val[M01] * val[M22] * val[M33] + val[M00] * val[M11] * val[M22] * val[M33];
}

bool Matrix4::inv() {
	float tmp[16];
	float l_det = det();
	if (l_det == 0) return false;
	tmp[M00] = val[M12] * val[M23] * val[M31] - val[M13] * val[M22] * val[M31] + val[M13] * val[M21] * val[M32] - val[M11]
		* val[M23] * val[M32] - val[M12] * val[M21] * val[M33] + val[M11] * val[M22] * val[M33];
	tmp[M01] = val[M03] * val[M22] * val[M31] - val[M02] * val[M23] * val[M31] - val[M03] * val[M21] * val[M32] + val[M01]
		* val[M23] * val[M32] + val[M02] * val[M21] * val[M33] - val[M01] * val[M22] * val[M33];
	tmp[M02] = val[M02] * val[M13] * val[M31] - val[M03] * val[M12] * val[M31] + val[M03] * val[M11] * val[M32] - val[M01]
		* val[M13] * val[M32] - val[M02] * val[M11] * val[M33] + val[M01] * val[M12] * val[M33];
	tmp[M03] = val[M03] * val[M12] * val[M21] - val[M02] * val[M13] * val[M21] - val[M03] * val[M11] * val[M22] + val[M01]
		* val[M13] * val[M22] + val[M02] * val[M11] * val[M23] - val[M01] * val[M12] * val[M23];
	tmp[M10] = val[M13] * val[M22] * val[M30] - val[M12] * val[M23] * val[M30] - val[M13] * val[M20] * val[M32] + val[M10]
		* val[M23] * val[M32] + val[M12] * val[M20] * val[M33] - val[M10] * val[M22] * val[M33];
	tmp[M11] = val[M02] * val[M23] * val[M30] - val[M03] * val[M22] * val[M30] + val[M03] * val[M20] * val[M32] - val[M00]
		* val[M23] * val[M32] - val[M02] * val[M20] * val[M33] + val[M00] * val[M22] * val[M33];
	tmp[M12] = val[M03] * val[M12] * val[M30] - val[M02] * val[M13] * val[M30] - val[M03] * val[M10] * val[M32] + val[M00]
		* val[M13] * val[M32] + val[M02] * val[M10] * val[M33] - val[M00] * val[M12] * val[M33];
	tmp[M13] = val[M02] * val[M13] * val[M20] - val[M03] * val[M12] * val[M20] + val[M03] * val[M10] * val[M22] - val[M00]
		* val[M13] * val[M22] - val[M02] * val[M10] * val[M23] + val[M00] * val[M12] * val[M23];
	tmp[M20] = val[M11] * val[M23] * val[M30] - val[M13] * val[M21] * val[M30] + val[M13] * val[M20] * val[M31] - val[M10]
		* val[M23] * val[M31] - val[M11] * val[M20] * val[M33] + val[M10] * val[M21] * val[M33];
	tmp[M21] = val[M03] * val[M21] * val[M30] - val[M01] * val[M23] * val[M30] - val[M03] * val[M20] * val[M31] + val[M00]
		* val[M23] * val[M31] + val[M01] * val[M20] * val[M33] - val[M00] * val[M21] * val[M33];
	tmp[M22] = val[M01] * val[M13] * val[M30] - val[M03] * val[M11] * val[M30] + val[M03] * val[M10] * val[M31] - val[M00]
		* val[M13] * val[M31] - val[M01] * val[M10] * val[M33] + val[M00] * val[M11] * val[M33];
	tmp[M23] = val[M03] * val[M11] * val[M20] - val[M01] * val[M13] * val[M20] - val[M03] * val[M10] * val[M21] + val[M00]
		* val[M13] * val[M21] + val[M01] * val[M10] * val[M23] - val[M00] * val[M11] * val[M23];
	tmp[M30] = val[M12] * val[M21] * val[M30] - val[M11] * val[M22] * val[M30] - val[M12] * val[M20] * val[M31] + val[M10]
		* val[M22] * val[M31] + val[M11] * val[M20] * val[M32] - val[M10] * val[M21] * val[M32];
	tmp[M31] = val[M01] * val[M22] * val[M30] - val[M02] * val[M21] * val[M30] + val[M02] * val[M20] * val[M31] - val[M00]
		* val[M22] * val[M31] - val[M01] * val[M20] * val[M32] + val[M00] * val[M21] * val[M32];
	tmp[M32] = val[M02] * val[M11] * val[M30] - val[M01] * val[M12] * val[M30] - val[M02] * val[M10] * val[M31] + val[M00]
		* val[M12] * val[M31] + val[M01] * val[M10] * val[M32] - val[M00] * val[M11] * val[M32];
	tmp[M33] = val[M01] * val[M12] * val[M20] - val[M02] * val[M11] * val[M20] + val[M02] * val[M10] * val[M21] - val[M00]
		* val[M12] * val[M21] - val[M01] * val[M10] * val[M22] + val[M00] * val[M11] * val[M22];
	float inv_det = 1.0f / l_det;
	val[M00] = tmp[M00] * inv_det;
	val[M01] = tmp[M01] * inv_det;
	val[M02] = tmp[M02] * inv_det;
	val[M03] = tmp[M03] * inv_det;
	val[M10] = tmp[M10] * inv_det;
	val[M11] = tmp[M11] * inv_det;
	val[M12] = tmp[M12] * inv_det;
	val[M13] = tmp[M13] * inv_det;
	val[M20] = tmp[M20] * inv_det;
	val[M21] = tmp[M21] * inv_det;
	val[M22] = tmp[M22] * inv_det;
	val[M23] = tmp[M23] * inv_det;
	val[M30] = tmp[M30] * inv_det;
	val[M31] = tmp[M31] * inv_det;
	val[M32] = tmp[M32] * inv_det;
	val[M33] = tmp[M33] * inv_det;
	return true;
}

void Matrix4::setToProjection(float _near, float _far, float _fovy, float _aspectRatio) {
	idt();
	float l_fd = (float)(1.f / std::tanf((_fovy * (b2_pi / 180.f)) / 2.f));
	float l_a1 = (_far + _near) / (_near - _far);
	float l_a2 = (2.f * _far * _near) / (_near - _far);
	val[M00] = l_fd / _aspectRatio;
	val[M10] = 0.f;
	val[M20] = 0.f;
	val[M30] = 0.f;
	val[M01] = 0.f;
	val[M11] = l_fd;
	val[M21] = 0.f;
	val[M31] = 0.f;
	val[M02] = 0.f;
	val[M12] = 0.f;
	val[M22] = l_a1;
	val[M32] = -1.f;
	val[M03] = 0.f;
	val[M13] = 0.f;
	val[M23] = l_a2;
	val[M33] = 0.f;
}

void Matrix4::setToProjection(float _left, float _right, float _bottom, float _top, float _near, float _far) {
	float x = 2.f * _near / (_right - _left);
	float y = 2.f * _near / (_top - _bottom);
	float a = (_right + _left) / (_right - _left);
	float b = (_top + _bottom) / (_top - _bottom);
	float l_a1 = (_far + _near) / (_near - _far);
	float l_a2 = (2.f * _far * _near) / (_near - _far);
	val[M00] = x;
	val[M10] = 0.f;
	val[M20] = 0.f;
	val[M30] = 0.f;
	val[M01] = 0.f;
	val[M11] = y;
	val[M21] = 0.f;
	val[M31] = 0.f;
	val[M02] = a;
	val[M12] = b;
	val[M22] = l_a1;
	val[M32] = -1.f;
	val[M03] = 0.f;
	val[M13] = 0.f;
	val[M23] = l_a2;
	val[M33] = 0.f;
}

void Matrix4::setToOrtho2D(float _x, float _y, float _width, float _height) {
	setToOrtho(_x, _x + _width, _y, _y + _height, 0.f, 1.f);
}

void Matrix4::setToOrtho2D(float _x, float _y, float _width, float _height, float _near, float _far) {
	setToOrtho(_x, _x + _width, _y, _y + _height, _near, _far);
}

void Matrix4::setToOrtho(float _left, float _right, float _bottom, float _top, float _near, float _far) {
	idt();
	float x_orth = 2.f / (_right - _left);
	float y_orth = 2.f / (_top - _bottom);
	float z_orth = -2.f / (_far - _near);

	float tx = -(_right + _left) / (_right - _left);
	float ty = -(_top + _bottom) / (_top - _bottom);
	float tz = -(_far + _near) / (_far - _near);

	val[M00] = x_orth;
	val[M10] = 0.f;
	val[M20] = 0.f;
	val[M30] = 0.f;
	val[M01] = 0.f;
	val[M11] = y_orth;
	val[M21] = 0.f;
	val[M31] = 0.f;
	val[M02] = 0.f;
	val[M12] = 0.f;
	val[M22] = z_orth;
	val[M32] = 0.f;
	val[M03] = tx;
	val[M13] = ty;
	val[M23] = tz;
	val[M33] = 1.f;
}

void Matrix4::setToLookAt(const sf::Vector3f& _direction, const sf::Vector3f& _up) {
	sf::Vector3f l_vex;
	sf::Vector3f l_vey;
	sf::Vector3f l_vez;

	l_vez = Main::nor(l_vez = _direction);
	l_vex = Main::nor(l_vex = _direction);
	l_vex = Main::crs(l_vex, _up);
	l_vex = Main::nor(l_vex);
	l_vey = Main::crs(l_vex, l_vez);
	l_vey = Main::nor(l_vey);

	idt();
	val[M00] = l_vex.x;
	val[M01] = l_vex.y;
	val[M02] = l_vex.z;
	val[M10] = l_vey.x;
	val[M11] = l_vey.y;
	val[M12] = l_vey.z;
	val[M20] = -l_vez.x;
	val[M21] = -l_vez.y;
	val[M22] = -l_vez.z;
}

void Matrix4::setToLookAt(const sf::Vector3f& _position, const sf::Vector3f& _target, const sf::Vector3f& _up) {
	sf::Vector3f tmpVec;
	tmpVec = _target - _position;
	setToLookAt(tmpVec, _up);
	Matrix4 tmp;
	tmp.setToTranslation(-_position.x, -_position.y, -_position.z);
	matrix4_mul(this->val, tmp.val);
}

void Matrix4::matrix4_proj(float* _mat, float* _vec) {
	float inv_w = 1.0f / (_vec[0] * _mat[M30] + _vec[1] * _mat[M31] + _vec[2] * _mat[M32] + _mat[M33]);
	float x = (_vec[0] * _mat[M00] + _vec[1] * _mat[M01] + _vec[2] * _mat[M02] + _mat[M03]) * inv_w;
	float y = (_vec[0] * _mat[M10] + _vec[1] * _mat[M11] + _vec[2] * _mat[M12] + _mat[M13]) * inv_w;
	float z = (_vec[0] * _mat[M20] + _vec[1] * _mat[M21] + _vec[2] * _mat[M22] + _mat[M23]) * inv_w;
	_vec[0] = x;
	_vec[1] = y;
	_vec[2] = z;
}

void Matrix4::proj(float* _mat, float* _vecs, int _offset, int _numVecs, int _stride) {
	float* vecPtr = _vecs + _offset;
	for (int i = 0; i < _numVecs; ++i) {
		matrix4_proj(_mat, vecPtr);
		vecPtr += _stride;
	}
}

Quaternion::Quaternion() {
	idt();
}

Quaternion::Quaternion(const Quaternion& _quat) {
	x = _quat.x;
	y = _quat.y;
	z = _quat.z;
	w = _quat.w;
}

void Quaternion::idt() {
	x = y = z = 0.f;
	w = 1.f;
}

Plane::Plane() : Plane(sf::Vector3(0.f, 0.f, 0.f), 0.f) {}

Plane::Plane(const sf::Vector3f& _normal, float _d) : normal(Main::nor(_normal)), d(_d){}

Plane::Plane(const Plane& _plane) : Plane(_plane.normal, _plane.d) {}

Plane::Plane(const sf::Vector3f& _normal, const sf::Vector3f& _point) : Plane(_normal, 
	-(_normal.x + _point.x + _normal.y + _point.y + _normal.z + _point.z)){}

Plane::Plane(const sf::Vector3f& _point1, const sf::Vector3f& _point2, const sf::Vector3f& _point3) {
	set(_point1, _point2, _point3);
}

void Plane::set(const Plane& _plane) {
	set(_plane.normal, _plane.d);
}

void Plane::set(const sf::Vector3f& _normal, float _d) {
	normal = sf::Vector3f(Main::nor(_normal));
	d = _d;
}

void Plane::set(const sf::Vector3f& _point1, const sf::Vector3f& _point2, const sf::Vector3f& _point3) {
	normal = _point1 - _point2;
	normal = Main::nor(Main::crs(normal, sf::Vector3f(_point2.x - _point3.x, _point2.y - _point3.y, _point2.z - _point3.z)));
	d = -(normal.x + _point1.x + normal.y + _point1.y + normal.z + _point1.z);
}

void Plane::set(float _nx, float _ny, float _nz, float _d) {
	normal = sf::Vector3f(_nx, _ny, _nz);
	d = _d;
}

float Plane::distance(const sf::Vector3f& _point) {
	return -(normal.x + _point.x + normal.y + _point.y + normal.z + _point.z) + d;
}

Plane::PlaneSide Plane::testPoint(const sf::Vector3f& _point) {
	float dist = distance(_point);
	if (Main::almost_equal(dist, 0.f))
		return Plane::PlaneSide::OnPlane;
	else if (dist < 0)
		return Plane::PlaneSide::Back;
	else
		return Plane::PlaneSide::Front;
}

Plane::PlaneSide Plane::testPoint(float _x, float _y, float _z) {
	return testPoint(sf::Vector3f(_x, _y, _z));
}

bool Plane::isFrontFacing(const sf::Vector3f& _point) {
	float dot = normal.x + _point.x + normal.y + _point.y + normal.z + _point.z;
	return dot < 0.f || Main::almost_equal(dot, 0.f);
}

BoundingBox::BoundingBox() {
	clr();
}

BoundingBox::BoundingBox(BoundingBox* _box) {
	set(_box);
}

BoundingBox::BoundingBox(const sf::Vector3f& _min, const sf::Vector3f& _max) {
	set(_min, _max);
}

void BoundingBox::set(BoundingBox* _box) {
	set(_box->min, _box->max);
}

void BoundingBox::set(const sf::Vector3f& _min, const sf::Vector3f& _max) {
	min = sf::Vector3f(std::min(_min.x, _max.x), std::min(_min.y, _max.y), std::min(_min.z, _max.z));
	max = sf::Vector3f(std::max(_min.x, _max.x), std::max(_min.y, _max.y), std::max(_min.z, _max.z));
	cnt = (min + max) * 0.5f;
	dim = max - min;
}

void BoundingBox::set(const std::vector<sf::Vector3f>& _points) {
	inf();
	for (auto& v : _points)
		ext(v);
}

void BoundingBox::inf() {
	min = sf::Vector3f(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
	max = sf::Vector3f(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());
	cnt = sf::Vector3f(0.f, 0.f, 0.f);
	dim = sf::Vector3f(0.f, 0.f, 0.f);
}

void BoundingBox::ext(const sf::Vector3f& _point) {
	set(sf::Vector3f(std::min(min.x, _point.x), std::min(min.y, _point.y), std::min(min.z, _point.z)), 
		sf::Vector3f(std::max(max.x, _point.x), std::max(max.y, _point.y), std::max(max.z, _point.z)));
}

void BoundingBox::clr() {
	set(sf::Vector3f(0.f, 0.f, 0.f), sf::Vector3f(0.f, 0.f, 0.f));
}

bool BoundingBox::isValid() {
	return min.x < max.x && Main::almost_equal(min.x, max.x) &&
		(min.y < max.y || Main::almost_equal(min.y, max.y)) &&
		(min.z < max.z || Main::almost_equal(min.z, max.z));
}

void BoundingBox::ext(BoundingBox* _box) {
	set(sf::Vector3f(std::min(min.x, _box->min.x), std::min(min.y, _box->min.y), std::min(min.z, _box->min.z)),
		sf::Vector3f(std::max(max.x, _box->max.x), std::max(max.y, _box->max.y), std::max(max.z, _box->max.z)));
}

void BoundingBox::ext(const sf::Vector3f& _centre, float _radius) {
	set(sf::Vector3f(std::min(min.x, _centre.x - _radius), std::min(min.y, _centre.y - _radius), std::min(min.z, _centre.z - _radius)),
		sf::Vector3f(std::max(max.x, _centre.x + _radius), std::max(max.y, _centre.y + _radius), std::max(max.z, _centre.z + _radius)));
}

void BoundingBox::ext(BoundingBox* _box, Matrix4* _transform) {
	ext(sf::Vector3f(_box->min.x, _box->min.y, _box->min.z) * _transform);
	ext(sf::Vector3f(_box->min.x, _box->min.y, _box->max.z) * _transform);
	ext(sf::Vector3f(_box->min.x, _box->max.y, _box->min.z) * _transform);
	ext(sf::Vector3f(_box->min.x, _box->max.y, _box->max.z) * _transform);
	ext(sf::Vector3f(_box->max.x, _box->min.y, _box->min.z) * _transform);
	ext(sf::Vector3f(_box->max.x, _box->min.y, _box->max.z) * _transform);
	ext(sf::Vector3f(_box->max.x, _box->max.y, _box->min.z) * _transform);
	ext(sf::Vector3f(_box->max.x, _box->max.y, _box->max.z) * _transform);
}

bool BoundingBox::contains(BoundingBox* _box) {
	return !isValid()
		|| ((min.x < _box->min.x || Main::almost_equal(min.x, _box->min.x)) &&
			(min.y < _box->min.y || Main::almost_equal(min.y, _box->min.y)) &&
			(min.z < _box->min.z || Main::almost_equal(min.z, _box->min.z)) &&
			(max.x > _box->max.x || Main::almost_equal(max.x, _box->max.x)) &&
			(max.y > _box->max.y || Main::almost_equal(max.y, _box->max.y)) &&
			(max.z > _box->max.z || Main::almost_equal(max.z, _box->max.z)));
}

bool BoundingBox::contains(const sf::Vector3f& _point) {
	return (min.x < _point.x || Main::almost_equal(min.x, _point.x)) &&
		(max.x > _point.x || Main::almost_equal(max.x, _point.x)) &&
		(min.y < _point.y || Main::almost_equal(min.y, _point.y)) &&
		(max.y > _point.y || Main::almost_equal(max.y, _point.y)) &&
		(min.z < _point.z || Main::almost_equal(min.z, _point.z)) &&
		(max.z > _point.z || Main::almost_equal(max.z, _point.z));
}

bool BoundingBox::intersects(BoundingBox* _box) {
	if (!isValid()) return false;

	// test using SAT (separating axis theorem)

	float lx = std::abs(cnt.x - _box->cnt.x);
	float sumx = (dim.x / 2.0f) + (_box->dim.x / 2.0f);

	float ly = std::abs(cnt.y - _box->cnt.y);
	float sumy = (dim.y / 2.0f) + (_box->dim.y / 2.0f);

	float lz = std::abs(cnt.z - _box->cnt.z);
	float sumz = (dim.z / 2.0f) + (_box->dim.z / 2.0f);

	return (lx <= sumx && ly <= sumy && lz <= sumz);
}

BoundingBox* BoundingBox::operator*= (Matrix4* _transform) {
	float x0 = min.x, y0 = min.y, z0 = min.z, x1 = max.x, y1 = max.y, z1 = max.z;
	inf();
	ext(sf::Vector3f(x0, y0, z0) * _transform);
	ext(sf::Vector3f(x0, y0, z1) * _transform);
	ext(sf::Vector3f(x0, y1, z0) * _transform);
	ext(sf::Vector3f(x0, y1, z1) * _transform);
	ext(sf::Vector3f(x1, y0, z0) * _transform);
	ext(sf::Vector3f(x1, y0, z1) * _transform);
	ext(sf::Vector3f(x1, y1, z0) * _transform);
	ext(sf::Vector3f(x1, y1, z1) * _transform);
	return this;
}

Frustum::Frustum() {
	clipSpacePlanePoints[0] = sf::Vector3f(-1.f, -1.f, -1.f);
	clipSpacePlanePoints[1] = sf::Vector3f(1.f, -1.f, -1.f);
	clipSpacePlanePoints[2] = sf::Vector3f(1.f, 1.f, -1.f);
	clipSpacePlanePoints[3] = sf::Vector3f(-1.f, 1.f, -1.f);
	clipSpacePlanePoints[4] = sf::Vector3f(-1.f, -1.f, 1.f);
	clipSpacePlanePoints[5] = sf::Vector3f(1.f, -1.f, 1.f);
	clipSpacePlanePoints[6] = sf::Vector3f(1.f, 1.f, 1.f);
	clipSpacePlanePoints[7] = sf::Vector3f(-1.f, 1.f, 1.f);

	int j = 0;
	for (auto& v : clipSpacePlanePoints) {
		clipSpacePlanePointsArray[j++] = v.x;
		clipSpacePlanePointsArray[j++] = v.y;
		clipSpacePlanePointsArray[j++] = v.z;
	}
}

void Frustum::update(Matrix4* _inverseProjectionView) {
	std::memcpy(_inverseProjectionView->val, planePointsArray, 24 * sizeof(float));
	Matrix4::proj(_inverseProjectionView->val, planePointsArray, 0, 8, 3);
	for (int i = 0, j = 0; i < 8; i++) {
		auto& v = planePoints[i];
		v.x = planePointsArray[j++];
		v.y = planePointsArray[j++];
		v.z = planePointsArray[j++];
	}

	planes[0].set(planePoints[1], planePoints[0], planePoints[2]);
	planes[1].set(planePoints[4], planePoints[5], planePoints[7]);
	planes[2].set(planePoints[0], planePoints[4], planePoints[3]);
	planes[3].set(planePoints[5], planePoints[1], planePoints[6]);
	planes[4].set(planePoints[2], planePoints[3], planePoints[6]);
	planes[5].set(planePoints[4], planePoints[0], planePoints[1]);
}

bool Frustum::pointInFrustum(const sf::Vector3f& _point) {
	return pointInFrustum(_point.x, _point.y, _point.z);
}

bool Frustum::pointInFrustum(float _x, float _y, float _z) {
	for (int i = 0; i < 6; ++i) {
		Plane::PlaneSide result = planes[i].testPoint(_x, _y, _z);
		if (result == Plane::PlaneSide::Back) return false;
	}
	return true;
}

bool Frustum::sphereInFrustum(const sf::Vector3f& _center, float _radius) {
	return sphereInFrustum(_center.x, _center.y, _center.z, _radius);
}

bool Frustum::sphereInFrustum(float _x, float _y, float _z, float _radius) {
	for (int i = 0; i < 6; ++i)
		if ((planes[i].normal.x * _x + planes[i].normal.y * _y + planes[i].normal.z * _z) < (-_radius - planes[i].d)) return false;
	return true;
}

bool Frustum::sphereInFrustumWithoutNearFar(const sf::Vector3f& _center, float _radius) {
	return sphereInFrustumWithoutNearFar(_center.x, _center.y, _center.z, _radius);
}

bool Frustum::sphereInFrustumWithoutNearFar(float _x, float _y, float _z, float _radius) {
	for (int i = 2; i < 6; ++i)
		if ((planes[i].normal.x * _x + planes[i].normal.y * _y + planes[i].normal.z * _z) < (-_radius - planes[i].d)) return false;
	return true;
}

bool Frustum::boundsInFrustum(BoundingBox* _box) {
	return boundsInFrustum(_box->cnt, _box->dim);
}

bool Frustum::boundsInFrustum(const sf::Vector3f& _center, const sf::Vector3f& _dim) {
	return boundsInFrustum(_center.x, _center.y, _center.z, _dim.x, _dim.y, _dim.z);
}

bool Frustum::boundsInFrustum(float _x, float _y, float _z, float _halfWidth, float _halfHeight, float _halfDepth) {
	for (int i = 0, len2 = 6; i < len2; ++i) {
		if (planes[i].testPoint(_x + _halfWidth, _y + _halfHeight, _z + _halfDepth) != Plane::PlaneSide::Back) continue;
		if (planes[i].testPoint(_x + _halfWidth, _y + _halfHeight, _z - _halfDepth) != Plane::PlaneSide::Back) continue;
		if (planes[i].testPoint(_x + _halfWidth, _y - _halfHeight, _z + _halfDepth) != Plane::PlaneSide::Back) continue;
		if (planes[i].testPoint(_x + _halfWidth, _y - _halfHeight, _z - _halfDepth) != Plane::PlaneSide::Back) continue;
		if (planes[i].testPoint(_x - _halfWidth, _y + _halfHeight, _z + _halfDepth) != Plane::PlaneSide::Back) continue;
		if (planes[i].testPoint(_x - _halfWidth, _y + _halfHeight, _z - _halfDepth) != Plane::PlaneSide::Back) continue;
		if (planes[i].testPoint(_x - _halfWidth, _y - _halfHeight, _z + _halfDepth) != Plane::PlaneSide::Back) continue;
		if (planes[i].testPoint(_x - _halfWidth, _y - _halfHeight, _z - _halfDepth) != Plane::PlaneSide::Back) continue;
		return false;
	}
	return true;
}
