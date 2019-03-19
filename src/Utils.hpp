#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	bool almost_equal(float _f1, float _f2); //TODO

	class BoundingBox2f {

	public:
		sf::Vector2f min;
		sf::Vector2f max;
		sf::Vector2f cnt;
		sf::Vector2f dim;

		void set(const sf::Vector2f&, const sf::Vector2f&);
		void set(const BoundingBox2f&);
		void inf();
		void ext(const sf::Vector2f&);
		void clr();
		bool isValid();
		void ext(const BoundingBox2f&);
		bool contains(const BoundingBox2f&);
		bool intersects(const BoundingBox2f&);
		bool contains(const sf::Vector2f&);
		void ext(float, float);
		BoundingBox2f();
		BoundingBox2f(sf::Vector2f, sf::Vector2f);

		bool operator==(const BoundingBox2f& _b) {
			return contains(_b);
		};

		bool operator==(const sf::Vector2f _v) {
			return contains(_v);
		};

		bool operator==(const sf::Vector2i _v) {
			return contains(sf::Vector2f(_v));
		};

		BoundingBox2f& operator+(const BoundingBox2f& _b) {
			ext(_b);
			return *this;
		}

		BoundingBox2f& operator+=(const BoundingBox2f& _b) {
			ext(_b);
			return *this;
		}

	};

	class VAO {
	protected:
		GLuint vao, vbo;
		GLint cameraUniformHandle;

		float* data;

		int vertexCount, vertexSize;

	public:
		virtual void set(float*, int) = 0;
		virtual void build(sf::Shader*) = 0;
		virtual void draw(sf::Shader*) = 0;

	};

	class BGVAO {

		GLuint texLoc[9];
		sf::Texture* tex[9];

		GLuint vao, vbo;
		GLint cameraUniformHandle;

		float* data;

		int vertexCount, vertexSize;

	public:
		void set(float*, int, int);
		void build(sf::Shader*);
		void draw(sf::Shader*);
	};

	class IndexedVAO {

		GLuint vao, vbo, index;
		GLint cameraUniformHandle, viewportSizeUniformHandle, radiusUniformHandle;

		float* data;
		GLuint* indices;

		int vertexCount, vertexSize;

		std::vector<GLuint> texLoc;
		std::vector<sf::Texture*> tex;

	public:
		float viewRadius = 500.f;

		void set(float*, GLuint*, int, int);
		void build(sf::Shader*);
		void draw(sf::Shader*);
	};

	class DebugDraw : public b2Draw {
	public:
		DebugDraw();
		void DrawPolygon(const b2Vec2*, int32, const b2Color&);
		void DrawSolidPolygon(const b2Vec2*, int32, const b2Color&);
		void DrawCircle(const b2Vec2&, float32, const b2Color&);
		void DrawSolidCircle(const b2Vec2&, float32, const b2Vec2&, const b2Color&);
		void DrawSegment(const b2Vec2&, const b2Vec2&, const b2Color&);
		void DrawTransform(const b2Transform&);
		void DrawPoint(const b2Vec2&, float32, const b2Color&);
		void DrawString(int, int, const char*);
		void DrawAABB(b2AABB*, const b2Color&);
		sf::Color B2SFColor(const b2Color&, int);
		void DrawMouseJoint(b2Vec2&, b2Vec2&, const b2Color&, const b2Color&);
	};

	class TextureAtlas;

	class SpriteBatch {


		const std::string fragment;

		sf::Shader* shader;

		int renderCalls = 0;
		int totalRenderCalls = 0;
		int maxSpritesInBatch = 0;
		int spriteCount = 0;

		std::atomic<bool> locked = false;
		bool isBlending = true;

		sf::Color color = sf::Color::White;

		std::queue< sf::Sprite*> drawQueue;
		std::mutex queueLock;

		std::thread* workthread;

		TextureAtlas* atlas;
		std::vector<GLuint> texLoc;
		std::unordered_map<GLuint, int> textures;

		void build();

		GLuint vao, vbo, index;
		float* data;
		int vertexCount;
		int vertexSize = 2 + 1 + 2 + 4; //pos + index + uv + color

		GLuint camLocation;

		void recompile(int);

	public:
		SpriteBatch(TextureAtlas*);
		SpriteBatch(TextureAtlas*, int);

		//builds the buffer asynchronous
		void begin();

		//draws the batch
		void end(sf::Transform&);

		inline void setColor(sf::Color _color) {
			color = _color;
		};

		//add to renderqueue
		void draw(sf::Sprite*);

		inline void enableBlending() {
			isBlending = true;
		};

		inline void disableBlending() {
			isBlending = false;
		};

		inline bool isDrawing() {
			return locked;
		}

		void setTextureAtlas(TextureAtlas*);
	};

}
