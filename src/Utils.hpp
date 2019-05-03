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

		std::vector<std::tuple<GLuint, float*>> buffers;
		uint currentIndex = 0;
		std::atomic<uint> spriteCount = 0;
		uint maxSpritesInBatch = 0;

		std::vector<GLuint> textures;
		std::unordered_map<uint, GLuint> textureMap;

		ShaderProgram* shader;

		void addSprite(const sf::FloatRect&, const sf::IntRect&, uint, const Vec2u&, const sf::Color&, const Vec4&);

		std::queue<Item*> drawJobs;
		void draw(Item*);
		void draw(void*, GLuint, sf::Color, const Vec4&);

		bool locked = false;

	public:
		SpriteBatch(uint);
		~SpriteBatch();

		const static uint VERTEXSIZE = 2 + 3 + 1 + 4; //pos, uv, col, scissor

		//builds the buffer asynchronous
		void build();

		//draws the batch 
		void drawToScreen();

		//add to renderqueue		
		void draw(sf::Sprite*, const Vec4&);
		void draw(sf::Sprite*, sf::Color);
		void draw(sf::Sprite*, sf::Color, const Vec4&);
		void draw(sf::Sprite*);

		void draw(Text::TextBlock*);
		void draw(Text::TextBlock*, Vec4);

		uint addTexture(GLuint);
		uint getIndex(GLuint);
		bool textureExists(GLuint);
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

		//Camera* cam = nullptr;

		GLuint vao, vbo;
		uint vertexCount = 0;
		const uint maxVertex;

		float* dataCache;

		void ver(const Vec4&, const sf::Color& = sf::Color::Black);
		void ver(float, float, float, float);

	public:

		ShapeRenderer(uint);

		float defaultRectLineWidth = 0.75f;

		void draw();
		
		void line(const Vec4&, const Vec4&, const sf::Color&);
		void line(const Vec4&, const sf::Color&, const Vec4&, const sf::Color&);

		void chain(const std::vector<Vec4>&, const sf::Color&);
		void chain(const std::vector<std::tuple<Vec4, sf::Color>>&);

		void loop(const std::vector<Vec4>&, const sf::Color&);
		void loop(const std::vector<std::tuple<Vec4, sf::Color>>&);

		void vertex(const Vec4&, const sf::Color&);
		void vertex(const std::vector<std::tuple<Vec4, sf::Color>>&);

		void triangle(const Vec4&, const Vec4&, const Vec4&, const sf::Color&);
		void triangle(const Vec4&, const sf::Color&, const Vec4&, const sf::Color&, const Vec4&, const sf::Color&);
		void triangle(const std::vector<std::tuple<Vec4, Vec4, Vec4>>&, const sf::Color&);
		void triangle(const std::vector<std::tuple<Vec4, Vec4, Vec4, sf::Color>>&);
		void triangle(const std::vector<std::tuple<Vec4, sf::Color, Vec4, sf::Color, Vec4, sf::Color >>&);

		void aabb(const Vec4&, float, const sf::Color&);
		void aabb(const Vec4&, const Vec4&, const sf::Color&);

		void circleXY(const Vec2&, float, const sf::Color&, uint = 24u);

		void sphere(const Vec4&, float, const sf::Color&, uint = 24u, uint = 10u);
		
		void polygon(const std::vector<Vec4>&, const sf::Color&);

		void draw(Ray*, const sf::Color& = sf::Color::Black);
		void draw(BoundingBox*, const sf::Color& = sf::Color::Black);
		void draw(Plane*, const sf::Color& = sf::Color::Black);
		void draw(Frustum*, Camera*, const sf::Color& = sf::Color::Black);
		void draw(Camera*, const sf::Color& = sf::Color::Black);

		void string(const std::string&, int, int, const sf::Color&);

	};

}
