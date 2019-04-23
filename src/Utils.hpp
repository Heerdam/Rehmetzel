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

	struct TextureAtlas;
	class FontCache;

	class SpriteBatch {

		friend Text::FontCache;
		friend Text::TextBlock;

		enum Type {
			sprite, font, static_font
		};

		struct Item {
			Item(Type _type, void* _data) : type(_type), data(_data) {};
			Type type;
			void* data;
		};

		struct DrawJob {
			Type type = Type::sprite;
			unsigned int count = 0;
			unsigned int offset = 0;
			Text::StaticTextBlock* block;
		};

		//used in work thread
		std::vector<DrawJob*> renderCache;
		std::queue<DrawJob*> renderQueue;

		std::condition_variable workThreadCv;
		std::condition_variable mainThreadCv;

		const std::string fragment;

		ShaderProgram* shader;

		int renderCalls = 0;
		int totalRenderCalls = 0;
		int maxSpritesInBatch = 0;

		std::atomic<int> spriteCount = 0;

		std::atomic<bool> terminate = false;
		std::atomic<bool> locked = false;
		bool isBlending = true;
		bool isDirty = true;

		sf::Color color = sf::Color::White;

		std::atomic_bool threadStatus[4]{ true, true, true, true };
		std::thread* workthread[4];
		std::vector<std::vector<Item*>> workCache = std::vector<std::vector<Item*>>(4);

		std::vector<GLuint> texLoc;
		std::vector<const sf::Texture*> texCache;
		std::unordered_map<GLuint, int> textures;

		void buildData(int);

		GLuint vao, vbo, index;
		float* data;
		int vertexCount;

		GLuint camLocation;
		GLuint widgetPositionLocation;

		void recompile(int);
		void compressDrawJobs(std::vector<DrawJob*>&);

		void draw(Item*);

	public:
		SpriteBatch(int, int);
		~SpriteBatch();

		//builds the buffer asynchronous
		void build();

		//draws the batch 
		void drawToScreen(const sf::Transform&);

		inline void setColor(sf::Color _color) {
			color = _color;
		};

		//add to renderqueue
		void draw(sf::Sprite*);
		void draw(Text::TextBlock*);
		void draw(Text::StaticTextBlock*);

		inline bool isDrawing() {
			return locked;
		}

		void addTexture(TextureAtlas*);
		void addTexture(const sf::Texture*);
		//for fonts textures needs to be pre initialized or manually added later
		void addTexture(sf::Font*);
		void addTexture(sf::Sprite*);
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

		sf::Color color = sf::Color::Black;
		Camera* cam = nullptr;

	public:

		float defaultRectLineWidth = 0.75f;

		//sphere
		unsigned int sectorCount = 24u;
		unsigned int stackCount = 10u;

		//circle
		unsigned int segments = 24u;

		void begin(Camera*);
		void end();
		void setColor(sf::Color);
		
		void x(const Vec3&, const Vec3&, float = 25.f);
		void arrow(const Vec3&, const Vec3&, float);
		void vertex(const Vec3&);
		void vertex(const std::vector<Vec3>&);
		void triangle(const Vec3&, const Vec3&, const Vec3&);
		void triangle(const std::vector<std::tuple<Vec3, Vec3, Vec3>>&);
		void line(const Vec3&, const Vec3&);
		void chain(const std::vector<Vec3>&);
		void loop(const std::vector<Vec3>&);
		void circle(const Vec3&, const Vec3&, float);
		void sphere(const Vec3&, float);
		void aabb(const Vec3&, float);
		void aabb(const Vec3&, const Vec3&);
		void polygon(const std::vector<Vec3>&);

		void draw(Ray*);
		void draw(BoundingBox*);
		void draw(Plane*);
		void draw(Frustum*);
		void draw(Camera*);

	};

}
