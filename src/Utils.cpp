
#include "Utils.hpp"
#include "Assets.hpp"
#include "CameraUtils.hpp"
#include "TextUtil.hpp"
#include "Math.hpp"
#include "TimeLog.hpp"

using namespace Heerbann;

SpriteBatch::SpriteBatch(uint _maxSprites) {
	
	buffers.resize(3);

	float* bp[3];
	GLuint b[3];
	glGenVertexArrays(3, b);

	uint bufferSize = 4 * VERTEXSIZE * _maxSprites * sizeof(float);

	for (uint i = 0; i < 3; ++i) {
		glBindBuffer(GL_ARRAY_BUFFER, b[i]);
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_CLIENT_STORAGE_BIT);
		bp[i] = reinterpret_cast<float*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, bufferSize,
			GL_MAP_READ_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_PERSISTENT_BIT));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	for (uint i = 0; i < 3; ++i)
		buffers[i] = std::make_tuple(b[i], bp[i]);

	shader = new ShaderProgram("/shader/spritebatch/sb_sprite");

}

Heerbann::SpriteBatch::~SpriteBatch() {}

void SpriteBatch::addSprite(const sf::FloatRect& _bounds, const sf::IntRect& _uv, uint _index, const Vec2u& _texBounds, const sf::Color& _tint, const Vec4& _scissors) {
	if (spriteCount >= maxSpritesInBatch) return;

	float color = M_FloatBits(_tint);

	float fracW = 1.f / _texBounds.x;
	float fracH = 1.f / _texBounds.y;

	uint count = spriteCount.fetch_add(1);
	int index = (currentIndex + 1) % 3;
	float* data = std::get<1>(buffers[index]) + count * 4 * VERTEXSIZE;

	//bottom left
	uint k = 0;
	data[k++] = _bounds.left; //x
	data[k++] = _bounds.top; //y

	data[k++] = static_cast<float>(_uv.left) * fracW; //u
	data[k++] = static_cast<float>(_uv.top) * fracH; //v
	data[k++] = static_cast<float>(_index); //i

	data[k++] = color; //col

	data[k++] = _scissors.x;
	data[k++] = _scissors.y;
	data[k++] = _scissors.z;
	data[k++] = _scissors.w;

	//bottom right
	data[k++] = _bounds.left + _bounds.width; //x
	data[k++] = _bounds.top; //y

	data[k++] = static_cast<float>(_uv.left + _uv.width) * fracW; //u
	data[k++] = static_cast<float>(_uv.top) * fracH; //v
	data[k++] = static_cast<float>(_index); //i

	data[k++] = color; //col

	data[k++] = _scissors.x;
	data[k++] = _scissors.y;
	data[k++] = _scissors.z;
	data[k++] = _scissors.w;

	//top left
	data[k++] = _bounds.left; //x
	data[k++] = _bounds.top + _bounds.height; //y

	data[k++] = static_cast<float>(_uv.left) * fracW; //u
	data[k++] = static_cast<float>(_uv.top + _uv.height) * fracH; //v
	data[k++] = static_cast<float>(_index); //i

	data[k++] = color; //col

	data[k++] = _scissors.x;
	data[k++] = _scissors.y;
	data[k++] = _scissors.z;
	data[k++] = _scissors.w;

	//top right
	data[k++] = _bounds.left + _bounds.width; //x
	data[k++] = _bounds.top + _bounds.height; //y

	data[k++] = static_cast<float>(_uv.left + _uv.width) * fracW; //u
	data[k++] = static_cast<float>(_uv.top + _uv.height) * fracH; //v
	data[k++] = static_cast<float>(_index); //i

	data[k++] = color; //col

	data[k++] = _scissors.x;
	data[k++] = _scissors.y;
	data[k++] = _scissors.z;
	data[k++] = _scissors.w;
}

void SpriteBatch::build() {
	spriteCount = 0;
	locked = true;
	while(!drawJobs.empty()) {
		Item* job = drawJobs.front();
		drawJobs.pop();
		switch (job->type) {
			case Type::sprite:
			{
				sf::Sprite* sprite = reinterpret_cast<sf::Sprite*>(job->data);
				auto tex = sprite->getTexture();
				addSprite(sprite->getGlobalBounds(), sprite->getTextureRect(), job->texIndex,
					Vec2u(tex->getSize().x, tex->getSize().y), job->color, job->scissors);
			}
			break;
			case Type::font:
			{
				int index = (currentIndex + 1) % 3;
				float* data = std::get<1>(buffers[index]) + spriteCount * 4 * VERTEXSIZE;
				Text::TextBlock* text = reinterpret_cast<Text::TextBlock*>(job->data);
				int count = 0;
				text->draw(count, data, job->scissors);
				spriteCount += count;
			}
			break;
		}
	}
}

void SpriteBatch::drawToScreen() {

	//thread kagge

	shader->bind();

	//bind textures
	glActiveTexture(GL_TEXTURE0);
	for(uint i = 0; i < textures.size(); ++i)
		glBindTexture(GL_TEXTURE + i, textures[i]);

	//bind current vbo
	glBindBuffer(GL_ARRAY_BUFFER, std::get<0>(buffers[currentIndex]));

	glEnableVertexAttribArray(0); //a_Pos
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, VERTEXSIZE * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1); //a_uv
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, VERTEXSIZE * sizeof(float), (void*)(2 * sizeof(float)));

	glEnableVertexAttribArray(2); //a_Col
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, VERTEXSIZE * sizeof(float), (void*)(5 * sizeof(float)));

	glEnableVertexAttribArray(3); //a_sci
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, VERTEXSIZE * sizeof(float), (void*)(6 * sizeof(float)));

	//draw
	glDrawArrays(GL_TRIANGLE_STRIP, 0, spriteCount * 4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE, 0);

	shader->unbind();

	currentIndex++;
}

void SpriteBatch::draw(Item* _item) {
	drawJobs.push(_item);
}

void SpriteBatch::draw(sf::Sprite* _sprite) {
	draw(_sprite, textureMap[_sprite->getTexture()->getNativeHandle()], sf::Color::White, Vec4(0.f, 0.f, M_WIDTH, M_HEIGHT));
}

void SpriteBatch::draw(void* _renderable, GLuint _texIndex, sf::Color _color, const Vec4& _scissors) {
	assert(!locked);
	draw(new Item {Type::sprite, _texIndex, _color, _scissors, _renderable });
}

void SpriteBatch::draw(sf::Sprite* _sprite, const Vec4& _scissors) {
	draw(_sprite, textureMap[_sprite->getTexture()->getNativeHandle()], sf::Color::White, _scissors);
}

void SpriteBatch::draw(sf::Sprite* _sprite, sf::Color _color) {
	draw(_sprite, textureMap[_sprite->getTexture()->getNativeHandle()], _color, Vec4(0.f, 0.f, M_WIDTH, M_HEIGHT));
}

void SpriteBatch::draw(sf::Sprite* _sprite, sf::Color _color, const Vec4& _scissors) {
	draw(_sprite, textureMap[_sprite->getTexture()->getNativeHandle()], _color, _scissors);
}

void SpriteBatch::draw(Text::TextBlock* _text) {
	draw(_text, Vec4(0.f, 0.f, M_WIDTH, M_HEIGHT));
}

void Heerbann::SpriteBatch::draw(Text::TextBlock* _text, Vec4 _scissors) {
	assert(!locked);
	draw(new Item{ Type::font, 0, sf::Color::White, _scissors, _text });
}

uint SpriteBatch::addTexture(GLuint _tex) {
	textures.emplace_back(_tex);
	uint index = 0;
	for (uint i = 0; i < textures.size(); ++i) {
		if (textures[i] == _tex) {
			index = i;
			break;
		}
	}
	textureMap[index] = _tex;
	return index;
}

uint SpriteBatch::getIndex(GLuint _handle) {
	if(textureExists(_handle)) return textureMap[_handle];
	else return addTexture(_handle);
}

bool SpriteBatch::textureExists(GLuint _handle) {
	return textureMap.count(_handle) > 0;
}

void ShapeRenderer::ver(const Vec4& _vertex, const sf::Color& _color) {
	ver(_vertex.x, _vertex.y, _vertex.z, M_FloatBits(_color));
}

void ShapeRenderer::ver(float _x, float _y, float _z, float _col) {
	if (vertexCount >= maxVertex)draw();
	dataCache[vertexCount] = _x;
	dataCache[vertexCount + 1] = _y;
	dataCache[vertexCount + 2] = _z;
	dataCache[vertexCount + 3] = _col;
	++vertexCount;
}

ShapeRenderer::ShapeRenderer(uint _maxVertex) : maxVertex(_maxVertex){}

void ShapeRenderer::draw() {
	shader->bind();
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount * vertexSize * sizeof(float), dataCache);
	glBindVertexArray(vao);
	glDrawArrays(GL_LINES, 0, vertexCount);
	vertexCount = 0;
	shader->unbind();
}

void ShapeRenderer::vertex(const Vec4& _pos, const sf::Color& _color) {
	float size = defaultRectLineWidth * 0.5f;
	line(Vec4(_pos.x - size, _pos.y - size, _pos.z, 1.f), Vec4(_pos.x + size, _pos.y + size, _pos.z, 1.f), _color);
}

void ShapeRenderer::vertex(const std::vector<std::tuple<Vec4, sf::Color>>& _vertices) {
	float size = defaultRectLineWidth * 0.5f;
	for(auto v : _vertices)
		line(Vec4(std::get<0>(v).x - size, std::get<0>(v).y - size, std::get<0>(v).z, 1.f),
			Vec4(std::get<0>(v).x + size, std::get<0>(v).y + size, std::get<0>(v).z, 1.f),
			std::get<1>(v));
}

void ShapeRenderer::triangle(const Vec4& _p1, const Vec4& _p2, const Vec4& _p3, const sf::Color& _col) {
	triangle(_p1, _col, _p2, _col, _p3, _col);
}

void ShapeRenderer::triangle(const Vec4& _p1, const sf::Color& _col1, const Vec4& _p2, const sf::Color& _col2, const Vec4& _p3, const sf::Color& _col3) {
	ver(_p1, _col1);
	ver(_p2, _col2);

	ver(_p2, _col2);
	ver(_p3, _col3);

	ver(_p3, _col3);
	ver(_p1, _col1);
}

void ShapeRenderer::triangle(const std::vector<std::tuple<Vec4, Vec4, Vec4>>& _triangles, const sf::Color& _col) {
	for (auto v : _triangles)
		triangle(std::get<0>(v), std::get<1>(v), std::get<2>(v), _col);
}

void ShapeRenderer::triangle(const std::vector<std::tuple<Vec4, Vec4, Vec4, sf::Color>>& _triangles) {
	for (auto v : _triangles)
		triangle(std::get<0>(v), std::get<1>(v), std::get<2>(v), std::get<3>(v));
}

void ShapeRenderer::line(const Vec4& _v1, const Vec4& _v2, const sf::Color& _col) {
	line(_v1, _col, _v2, _col);
}

void ShapeRenderer::line(const Vec4& _v1, const sf::Color& _col1, const Vec4& _v2, const sf::Color& _col2) {
	ver(_v1, _col1);
	ver(_v2, _col2);
}

void ShapeRenderer::chain(const std::vector<Vec4>& _points, const sf::Color& _col) {
	if (_points.size() < 2) return;
	for (unsigned int i = 0; i < _points.size() - 1; ++i)
		line(_points[i], _points[i + 1], _col);
}

void ShapeRenderer::chain(const std::vector<std::tuple<Vec4, sf::Color>>& _points) {
	if (_points.size() < 2) return;
	Vec4 vec = std::get<0>(_points[1]);
	sf::Color col = std::get<1>(_points[1]);
	line(std::get<0>(_points[0]), std::get<1>(_points[1]), vec, col);
	for (unsigned int i = 2; i < _points.size() - 1; ++i) {
		line(vec, col, std::get<0>(_points[i]), std::get<1>(_points[i]));
		vec = std::get<0>(_points[i]);
		col = std::get<1>(_points[i]);
	}
}

void ShapeRenderer::loop(const std::vector<Vec4>& _points, const sf::Color& _col) {
	if (_points.size() < 2) return;
	chain(_points, _col);
	line(_points[_points.size()-1], _points[0], _col);
}

void ShapeRenderer::loop(const std::vector<std::tuple<Vec4, sf::Color>>& _points) {
	chain(_points);
	line(std::get<0>(_points[_points.size() - 1]), std::get<1>(_points[_points.size() - 1]), std::get<0>(_points[0]), std::get<1>(_points[0]));
}

void ShapeRenderer::circleXY(const Vec2& _center, float _radius, const sf::Color& _color, uint _segments) {
	std::vector<Vec4> vertices(_segments);
	float angle = 2.f * PI / static_cast<float>(_segments);
	float cos = COS(angle);
	float sin = SIN(angle);
	float cx = _radius, cy = 0.f;

	for (uint i = 0; i < _segments; i++) {
		vertices.push_back(Vec4(_center.x + cx, _center.y + cy, 0, 1.f));
		float temp = cx;
		cx = cos * cx - sin * cy;
		cy = sin * temp + cos * cy;
		vertices.push_back(Vec4(_center.x + cx, _center.y + cy, 0, 1.f));
	}
	// Ensure the last segment is identical to the first.
	vertices.push_back(Vec4(_center.x + cx, _center.y + cy, 0, 1.f));

	float temp = cx;
	cx = _radius;
	cy = 0.f;
	vertices.push_back(Vec4(_center.x + cx, _center.y + cy, 0, 1.f));

	loop(vertices, _color);
}

void ShapeRenderer::sphere(const Vec4& _center, float _radius, const sf::Color& _color, uint _sectorCount, uint _stackCount) {

	std::vector<Vec4> vertices (_stackCount * _sectorCount);

	float x, y, z, xy;                              // vertex position

	float sectorStep = 2 * PI / _sectorCount;
	float stackStep = PI / _stackCount;
	float sectorAngle, stackAngle;

	for (uint i = 0; i <= _stackCount; ++i) {
		stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
		xy = _radius * cosf(stackAngle);             // r * cos(u)
		z = _radius * sinf(stackAngle);              // r * sin(u)

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for (uint j = 0; j <= _sectorCount; ++j) {
			sectorAngle = j * sectorStep;           // starting from 0 to 2pi

			// vertex position (x, y, z)
			x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
			y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)

			vertices.push_back(Vec4(x, y, z, 1.f));

		}
	}

	int k1, k2;
	for (uint i = 0; i < _stackCount; ++i) {
		k1 = i * (_sectorCount + 1);     // beginning of current stack
		k2 = k1 + _sectorCount + 1;      // beginning of next stack

		for (uint j = 0; j < _sectorCount; ++j, ++k1, ++k2) {
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if (i != 0)
				triangle(vertices[k1], vertices[k2], vertices[k1 + 1], _color);

			// k1+1 => k2 => k2+1
			if (i != (_stackCount - 1))
				triangle(vertices[k1 + 1], vertices[k2], vertices[k2 + 1], _color);
		}
	}

}

void ShapeRenderer::aabb(const Vec4& _center, float _side, const sf::Color& _color) {
	Vec4 hs = Vec4(_side, _side, _side, 1.f) * 0.5f;
	aabb(_center - hs, _center + hs, _color);
}

void ShapeRenderer::aabb(const Vec4& _min, const Vec4& _max, const sf::Color& _col) {
	//x
	ver(_min.x, _min.y, _min.z, M_FloatBits(sf::Color::Red));
	ver(_max.x, _min.y, _min.z, M_FloatBits(sf::Color::Red));

	//y
	ver(_min.x, _min.y, _min.z, M_FloatBits(sf::Color::Green));
	ver(_min.x, _max.y, _min.z, M_FloatBits(sf::Color::Green));

	//z
	ver(_min.x, _min.y, _max.z, M_FloatBits(sf::Color::Blue));
	ver(_min.x, _min.y, _min.z, M_FloatBits(sf::Color::Blue));

	ver(_max.x, _min.y, _min.z, M_FloatBits(_col));
	ver(_max.x, _max.y, _min.z, M_FloatBits(_col));

	ver(_max.x, _min.y, _max.z, M_FloatBits(_col));
	ver(_max.x, _max.y, _max.z, M_FloatBits(_col));

	ver(_min.x, _min.y, _max.z, M_FloatBits(_col));
	ver(_max.x, _max.y, _min.z, M_FloatBits(_col));

	ver(_max.x, _min.y, _min.z, M_FloatBits(_col));
	ver(_max.x, _min.y, _max.z, M_FloatBits(_col));

	ver(_max.x, _min.y, _max.z, M_FloatBits(_col));
	ver(_min.x, _min.y, _max.z, M_FloatBits(_col));

	ver(_min.x, _max.y, _min.z, M_FloatBits(_col));
	ver(_max.x, _max.y, _min.z, M_FloatBits(_col));

	ver(_max.x, _max.y, _min.z, M_FloatBits(_col));
	ver(_max.x, _max.y, _max.z, M_FloatBits(_col));

	ver(_max.x, _max.y, _max.z, M_FloatBits(_col));
	ver(_max.x, _max.y, _min.z, M_FloatBits(_col));

	ver(_max.x, _max.y, _min.z, M_FloatBits(_col));
	ver(_min.x, _max.y, _min.z, M_FloatBits(_col));
}

void ShapeRenderer::polygon(const std::vector<Vec4>& _points, const sf::Color& _col) {
	loop(_points, _col);
}

void ShapeRenderer::draw(Ray* _ray, const sf::Color& _col) {
	line(_ray->origin, _col, _ray->getEndPoint(1000.f), sf::Color::Transparent);
}

void ShapeRenderer::draw(BoundingBox* _bounds, const sf::Color& _col) {
	aabb(_bounds->min, _bounds->max, _col);
}

void ShapeRenderer::draw(Plane* _plane, const sf::Color& _col) {
	//TODO: nicht wirklich sinnvoll?
}

void ShapeRenderer::draw(Frustum* _frustum, Camera* _cam, const sf::Color& _col) {
	auto points = _frustum->getPoints(_cam);
	float color = M_FloatBits(_col);

	//front square
	ver(points[0].x, points[0].y, points[0].z, color);
	ver(points[1].x, points[1].y, points[1].z, color);

	ver(points[1].x, points[1].y, points[1].z, color);
	ver(points[2].x, points[2].y, points[2].z, color);

	ver(points[2].x, points[2].y, points[2].z, color);
	ver(points[3].x, points[3].y, points[3].z, color);

	ver(points[3].x, points[3].y, points[3].z, color);
	ver(points[0].x, points[0].y, points[0].z, color);

	//back square
	ver(points[4].x, points[4].y, points[4].z, color);
	ver(points[5].x, points[5].y, points[5].z, color);

	ver(points[5].x, points[5].y, points[5].z, color);
	ver(points[6].x, points[6].y, points[6].z, color);

	ver(points[6].x, points[6].y, points[6].z, color);
	ver(points[7].x, points[7].y, points[7].z, color);

	ver(points[7].x, points[7].y, points[7].z, color);
	ver(points[4].x, points[4].y, points[4].z, color);

	//side lines
	ver(points[0].x, points[0].y, points[0].z, color);
	ver(points[4].x, points[4].y, points[4].z, color);

	ver(points[1].x, points[1].y, points[1].z, color);
	ver(points[5].x, points[5].y, points[5].z, color);

	ver(points[2].x, points[2].y, points[2].z, color);
	ver(points[6].x, points[6].y, points[6].z, color);

	ver(points[3].x, points[3].y, points[3].z, color);
	ver(points[7].x, points[7].y, points[7].z, color);

}

void ShapeRenderer::draw(Camera* _cam, const sf::Color& _col) {
	draw(_cam->frustum, _cam, _col);
}

void ShapeRenderer::string(const std::string &, int, int, const sf::Color&) {}
