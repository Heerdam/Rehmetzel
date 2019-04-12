#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

#define VERTEXSIZE 8 //pos + index + typ + uv + color1 + color2
#define MAXSPRITES 1000
#define TYP_SPRITE 0.f
#define TYP_FONT 1.f
#define TYP_LINE 2.f
#define TYP_BOX 3.f
#define TYP_FONT_STATIC 4.f

	namespace Text {
		class TextBlock;
		class FontCache;
		struct StaticTextBlock;
	}

	class ShaderProgram;
	class Quaternion;
	class Matrix4;

	struct Plane {
		enum PlaneSide {
			OnPlane, Back, Front
		};

		sf::Vector3f normal;
		float d; //The distance to the origin

		Plane();
		Plane(const sf::Vector3f&, float);
		Plane(const Plane&);
		Plane(const sf::Vector3f&, const sf::Vector3f&);
		Plane(const sf::Vector3f&, const sf::Vector3f&, const sf::Vector3f&);

		void set(const Plane&);
		void set(const sf::Vector3f&, float);
		void set(const sf::Vector3f&, const sf::Vector3f&, const sf::Vector3f&);
		void set(float, float, float, float);

		float distance(const sf::Vector3f&);

		PlaneSide testPoint(const sf::Vector3f&);
		PlaneSide testPoint(float, float, float);

		bool isFrontFacing(const sf::Vector3f&);

	};

	struct BoundingBox {
		sf::Vector3f min, max;
		sf::Vector3f cnt, dim;

		BoundingBox();
		BoundingBox(BoundingBox*);
		BoundingBox(const sf::Vector3f&, const sf::Vector3f&);

		void set(BoundingBox*);
		void set(const sf::Vector3f&, const sf::Vector3f&);
		void set(const std::vector<sf::Vector3f>&);

		void inf();
		void ext(const sf::Vector3f&);
		void clr();
		bool isValid();

		void ext(BoundingBox*);
		void ext(const sf::Vector3f&, float);
		void ext(BoundingBox*, Matrix4*);

		BoundingBox* operator*= (Matrix4*);

		bool contains(BoundingBox*);
		bool contains(const sf::Vector3f&);

		bool intersects(BoundingBox*);
	};

	class Frustum {

		sf::Vector3f clipSpacePlanePoints[8];
		float clipSpacePlanePointsArray[24];

		Plane planes[6];

		sf::Vector3f planePoints[8];
		float planePointsArray[24];

		Frustum();

		void update(Matrix4*);

		bool pointInFrustum(const sf::Vector3f&);
		bool pointInFrustum(float, float, float);

		bool sphereInFrustum(const sf::Vector3f&, float);
		bool sphereInFrustum(float, float, float, float);

		bool sphereInFrustumWithoutNearFar(const sf::Vector3f&, float);
		bool sphereInFrustumWithoutNearFar(float, float, float, float);

		bool boundsInFrustum(BoundingBox*);
		bool boundsInFrustum(const sf::Vector3f&, const sf::Vector3f&);
		bool boundsInFrustum(float, float, float, float, float, float);

	};

	class Matrix4 {

	public:
		static const int M00 = 0;
		static const int M01 = 4;
		static const int M02 = 8;
		static const int M03 = 12;
		static const int M10 = 1;
		static const int M11 = 5;
		static const int M12 = 9;
		static const int M13 = 13;
		static const int M20 = 2;
		static const int M21 = 6;
		static const int M22 = 10;
		static const int M23 = 14;
		static const int M30 = 3;
		static const int M31 = 7;
		static const int M32 = 11;
		static const int M33 = 15;

	private:

		static void matrix4_mul(float* mata, float* matb);		
		float tmp[16]{ 0 };

	public:
		float val[16]{ 0 };

		Matrix4();
		Matrix4(Matrix4*);
		Matrix4(Quaternion*);
		Matrix4(const sf::Vector3f&, Quaternion*, const sf::Vector3f&);
		Matrix4(const aiVector3D&, const aiQuaternion&, const aiVector3D&);

		Matrix4* operator=(Matrix4*);
		Matrix4* operator=(Quaternion*);
		Matrix4* operator*(Matrix4*);

		Matrix4* set(const sf::Vector3f&, Quaternion*, const sf::Vector3f&);
		Matrix4* set(float, float, float, float, float, float, float, float, float, float);	
		Matrix4* set(float*);
		void setToTranslation(const sf::Vector3f&);
		void setToTranslation(float, float, float);

		float operator[](int);

		//Transposes the matrix.
		void tra();
		void idt();
		float det();
		bool inv();

		/*
		Sets the matrix to a projection matrix with a near- and far plane, a field of view in degrees and an aspect ratio. Note that
		the field of view specified is the angle in degrees for the height, the field of view for the width will be calculated
		according to the aspect ratio.

		near: The near plane
		far: The far plane
		fovy: The field of view of the height in degrees
		aspectRatio: The "width over height" aspect ratio
		return: This matrix for the purpose of chaining methods together. 
		 */
		void setToProjection(float, float, float, float);

		/*
		Sets the matrix to a projection matrix with a near/far plane, and left, bottom, right and top specifying the points on the
		near plane that are mapped to the lower left and upper right corners of the viewport. This allows to create projection
		matrix with off-center vanishing point.

		near: The near plane
		far: The far plane
		return: This matrix for the purpose of chaining methods together. 
		*/
		void setToProjection(float, float, float, float, float, float);

		/*
		Sets this matrix to an orthographic projection matrix with the origin at (x,y) extending by width and height. The near plane
		is set to 0, the far plane is set to 1.

		x: The x-coordinate of the origin
		y: The y-coordinate of the origin
		width: The width
		height: The height
		return: This matrix for the purpose of chaining methods together. 
		*/
		void setToOrtho2D(float, float, float, float);

		/*
		Sets this matrix to an orthographic projection matrix with the origin at (x,y) extending by width and height, having a near
		and far plane.

		x: The x-coordinate of the origin
		y: The y-coordinate of the origin
		width: The width
		height: The height
		near: The near plane
		far: The far plane
		return: This matrix for the purpose of chaining methods together. 
		*/
		void setToOrtho2D(float, float, float, float, float, float);

		/*
		Sets the matrix to an orthographic projection like glOrtho (http://www.opengl.org/sdk/docs/man/xhtml/glOrtho.xml) following
		the OpenGL equivalent
	
		left The left clipping plane
		right The right clipping plane
		bottom The bottom clipping plane
		top The top clipping plane
		near The near clipping plane
		far The far clipping plane
		return This matrix for the purpose of chaining methods together. 
		*/
		void setToOrtho(float, float, float, float, float, float);

		void setToLookAt(const sf::Vector3f&, const sf::Vector3f&);
		void setToLookAt(const sf::Vector3f&, const sf::Vector3f&, const sf::Vector3f&);
	
		static void proj(float*, float*, int, int, int);
		static void matrix4_proj(float*, float*);
};

	class Quaternion {
	public:
		Quaternion();
		Quaternion(const Quaternion&);
		float x, y, z, w;
		void idt();
	};

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

	class ShaderProgram {
		enum Status {
			success, failed, missing
		};
		GLuint program, compute, vertex, geom, frag;
		void print(std::string, Status, Status, Status, Status, Status, std::string);
		bool compile(const std::string&, const char*, const char*, const char*, const char*);
	public:
		bool printDebug = true;
		GLuint getHandle();
		bool loadFromMemory(const std::string&, const std::string&, const std::string&, const std::string&, const std::string&);
		void bind();
		void unbind();
	};

}
