#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	class VAO {
	protected:
		GLuint vao, vbo;
		GLint cameraUniformHandle;

		float* data;

		int vertexCount, vertexSize;

	public:
		virtual void set(float*, int) = 0;
		virtual void build(ShaderProgram*) = 0;
		virtual void draw(ShaderProgram*) = 0;

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
		void build(ShaderProgram*);
		void draw(ShaderProgram*);
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
		void build(ShaderProgram*);
		void draw(ShaderProgram*);
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

	class SpriteBatch {

		enum Type {
			sprite, font
		};

		struct Item {
			Type type;
			uint texIndex;
			sf::Color color;
			Vec4 scissors;
			void* data;
		};

		const uint VERTEXSIZE = 2 + 3 + 1 + 4; //pos, uv, col, scissor

		std::vector<std::tuple<GLuint, float*>> buffers;
		uint currentIndex = 0;
		std::atomic<uint> spriteCount = 0;
		uint maxSpritesInBatch = 0;

		std::vector<GLuint> textures;
		std::unordered_map<uint, GLuint> textureMap;

		ShaderProgram* shader;

		std::vector<std::thread*> workthreads;

		void addSprite(const sf::FloatRect&, const sf::FloatRect&, const Vec2&, uint, const sf::Color& = sf::Color::White, const Vec4& = Vec4(0.f, 0.f, M_WIDTH, M_HEIGHT));


		void draw(Item*);
		void draw(void*, GLuint, sf::Color, const Vec4&);

		bool locked = false;

	public:
		SpriteBatch(uint);
		~SpriteBatch();

		//builds the buffer asynchronous
		void build();

		//draws the batch 
		void drawToScreen(GLuint);

		//add to renderqueue		
		void draw(sf::Sprite*, const Vec4&);
		void draw(sf::Sprite*, sf::Color);
		void draw(sf::Sprite*, sf::Color, const Vec4&);
		void draw(sf::Sprite*);

		void draw(Text::TextBlock*);

		uint addTexture(GLuint);
	};

	class ShaderProgram {
		enum Status {
			success, failed, missing
		};
		GLuint program = -1, compute = -1, vertex = -1, geom = -1, frag = -1;
		void print(std::string, Status, Status, Status, Status, Status, std::string);
		bool compile(const std::string&, const char*, const char*, const char*, const char*);
	public:
		bool printDebug = true;
		GLuint getHandle();
		bool loadFromMemory(const std::string&, const std::string&, const std::string&, const std::string&, const std::string&);
		void bind();
		void unbind();
	};

	class ShapeRenderer {

		const uint vertexSize = 3 + 1;

		ShaderProgram* shader;

		Camera* cam = nullptr;

		GLuint vao, vbo;
		uint vertexCount = 0;

		float* dataCache;

		void ver(const Vec3&, const sf::Color& = sf::Color::Black);
		void ver(float, float, float, float);

	public:

		float defaultRectLineWidth = 0.75f;

		void begin(Camera*);
		void end();
		void flush();
		
		void line(const Vec3&, const Vec3&, const sf::Color&);
		void line(const Vec3&, const sf::Color&, const Vec3&, const sf::Color&);

		void chain(const std::vector<Vec3>&, const sf::Color&);
		void chain(const std::vector<std::tuple<Vec3, sf::Color>>&);

		void loop(const std::vector<Vec3>&, const sf::Color&);
		void loop(const std::vector<std::tuple<Vec3, sf::Color>>&);

		void vertex(const Vec3&, const sf::Color&);
		void vertex(const std::vector<std::tuple<Vec3, sf::Color>>&);

		void triangle(const Vec3&, const Vec3&, const Vec3&, const sf::Color&);
		void triangle(const Vec3&, const sf::Color&, const Vec3&, const sf::Color&, const Vec3&, const sf::Color&);
		void triangle(const std::vector<std::tuple<Vec3, Vec3, Vec3>>&, const sf::Color&);
		void triangle(const std::vector<std::tuple<Vec3, Vec3, Vec3, sf::Color>>&);
		void triangle(const std::vector<std::tuple<Vec3, sf::Color, Vec3, sf::Color, Vec3, sf::Color >>&);

		void aabb(const Vec3&, float, const sf::Color&);
		void aabb(const Vec3&, const Vec3&, const sf::Color&);

		void circleXY(const Vec2&, float, const sf::Color&, uint = 24u);

		void sphere(const Vec3&, float, const sf::Color&, uint = 24u, uint = 10u);
		
		void polygon(const std::vector<Vec3>&, const sf::Color&);

		void draw(Ray*, const sf::Color& = sf::Color::Black);
		void draw(BoundingBox*, const sf::Color& = sf::Color::Black);
		void draw(Plane*, const sf::Color& = sf::Color::Black);
		void draw(Frustum*, Camera*, const sf::Color& = sf::Color::Black);
		void draw(Camera*, const sf::Color& = sf::Color::Black);

		void string(const std::string&, int, int, const sf::Color&);

	};

}
