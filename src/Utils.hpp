#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

#define VERTEXSIZE 8 //pos + index + typ + uv + color1 + color2
#define TYP_SPRITE 0.f
#define TYP_FONT 1.f

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

	struct TextureAtlas;
	class FontCache;

	class SpriteBatch {

		friend FontCache;

		enum Type {
			sprite, font
		};

		struct Item {
			Item(Type _type, void* _data) : type(_type), data(_data) {};
			Type type;
			void* data;
		};

		const std::string fragment;

		sf::Shader* shader;

		int renderCalls = 0;
		int totalRenderCalls = 0;
		int maxSpritesInBatch = 0;

		std::atomic<int> spriteCount = 0;

		std::atomic<bool> locked = false;
		bool isBlending = true;
		bool isDirty = true;

		sf::Color color = sf::Color::White;

		std::vector<Item*> drawQueue;
		std::mutex queueLock;

		std::thread* workthread[5];

		std::vector<GLuint> texLoc;
		std::vector<const sf::Texture*> texCache;
		std::unordered_map<GLuint, int> textures;

		void buildData(std::vector<Item*>::iterator, std::vector<Item*>::iterator);

		GLuint vao, vbo, index;
		float* data;
		int vertexCount;

		GLuint camLocation;

		void recompile(int);

	public:
		SpriteBatch(int, int);

		//builds the buffer asynchronous
		void build();

		//draws the batch
		void drawToScreen(const sf::Transform&);

		inline void setColor(sf::Color _color) {
			color = _color;
		};

		//add to renderqueue
		void draw(sf::Sprite*);
		void draw(FontCache*);

		inline void enableBlending() {
			isBlending = true;
		};

		inline void disableBlending() {
			isBlending = false;
		};

		inline bool isDrawing() {
			return locked;
		}

		void addTexture(TextureAtlas*);
		void addTexture(const sf::Texture*);
		//for fonts textures needs to be pre initialized or manually added later
		void addTexture(sf::Font*);
		void addTexture(sf::Sprite*);
	};

	class FontCache {

		/*
		allowed markups:
		font: {fn=font name}
		color: {fc=rrrgggbbbaaa}
		border color: {bc=rrrgggbbbaaa}
		border thickness: {bt=float}
		size: {sz=uint}
		bold: {bl=true}
		combination: {fn=font name,fc=rrrgggbbbaaa,sz=uint}
		end of style: {en}
		*/

		enum Align {
			left, centre
		};

		struct Letter {
			Letter() {};
			Letter(const Letter&);
			uint32 letter;
			unsigned int size;
			float ot; //outline thickness
			sf::Color color;
			sf::Color oColor; //outline color
			sf::Vector2f pos; //relative to line
			int texIndex;
			bool bold;
			sf::Font* font;
			sf::Glyph* glyph;
		};

		struct Line {
			Line(float _maxWidth) : maxWidth(_maxWidth) {};
			std::vector<Letter*> letters;
			float spacing = std::numeric_limits<float>::infinity();
			sf::Vector2f pos; //relative to origin
			float maxWidth;
			//return false if line full
			bool insert(float&, Letter*);
		};

		struct TextBlock {
			std::vector<Line*> lines;
			Align align = Align::left;
			sf::Vector2f pos; //origin			
		};

		struct Style {
			Style() {};
			Style(Style&);
			Align align = Align::left;
			sf::Font* font = Main::getDefaultFont();
			sf::Color fontColor = sf::Color::White;
			sf::Color outlineColor = sf::Color(71, 71, 71, 255);
			unsigned int fontSize = MEDIUMFONTSIZE;
			float outlineThickness = 0.f;
			bool bold = false;
		};

		TextBlock block;

		std::wstring text;
		bool isDirty;

		float* cache;
		int size = 0;

		void layout();
		void build();

		std::unordered_map<std::wstring, sf::Font*> fonts;

	public:

		sf::Vector2f pos;
		float width = 200;
		Style defaultStyle;
		

		FontCache();

		void addFont(std::wstring, sf::Font*);

		void setText(std::wstring);

		//thread safe. only called by spritebatch
		float* draw(int&);
		
	};

	class SplitFunctor {
	public:
		SplitFunctor(std::wregex _splitter, std::wstring _original)
			: splitter{ std::move(_splitter) }
			, original{ std::move(_original) }
		{};

		auto begin() const {
			return std::wsregex_token_iterator{ original.begin(), original.end(), splitter, -1 };
		};

		auto end() const {
			return std::wsregex_token_iterator{};
		};

		template <typename Container>
		operator Container () const {
			return { begin(), end() };
		};

	private:
		std::wregex splitter;
		std::wstring original;
	};

}
