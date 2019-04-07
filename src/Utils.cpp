
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
