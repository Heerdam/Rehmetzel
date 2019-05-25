#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

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

};


