#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	using namespace Heerbann;

	enum Type {
		byteArray, image, texture2D, texture2DArray, 
		font, level, atlas, shader, model, renderer,
		shadowMap, framebuffer
	};

	struct Ressource {

		friend AssetManager;

	protected:
		std::atomic<bool> isLoaded = false;

		uint dataSize;
		void* data;

		virtual void load() {};
		virtual void unload() {};
		virtual void glLoad(void*) {};
		virtual void glUnload(void*) {};

	public:

		const Type type;
		const std::string id;

		Ressource(std::string, Type);
		~Ressource();

		bool inline loaded() {
			return isLoaded;
		}
	};

	class AssetManager {

		friend Ressource;

		std::unordered_map<std::string, Ressource*> assets;

		void loadFromDisk(std::string, Ressource*);
		void unload(Ressource*);

	public:

		template<class T>
		T get(std::string);

		bool exists(std::string);
	};

	template<class T>
	inline T AssetManager::get(std::string _id) {
		if (!exists(_id)) return nullptr;
		return reinterpret_cast<T>(assets[_id]);
	}

	class Image : public Ressource {
		sf::Image* image;
	public:
		Image(std::string);
		void load() override;
		void unload() override;
		sf::Image* get();
		void finish();
		static Image* get(std::string);
	};

	class Texture2D : public Ressource {
		friend TextureDebugRenderer;
		friend Framebuffer;

		GLuint handle;
		GLenum target, format, type;
		GLint level, internalFormat;
		Vec2u bounds;
	protected:
		void load() override;
		void glLoad(void*) override;
		void glUnload(void*) override;
	public:
		//https://www.khronos.org/opengl/wiki/GLAPI/glTexImage2D
		//target, level, internalFormat, format, type
		Texture2D(std::string, GLuint, GLint, GLint, GLenum, GLenum);
		GLuint get();
		void bind(GLuint);
		void setWrap(GLint, GLint);
		void setFilter(GLint, GLint);
		void setParai(GLenum, GLint);
		void setParaf(GLenum, GLfloat);
		static Texture2D* get(std::string);
	};

	class Array2DTexture : public Ressource {
		friend TextureDebugRenderer;

		std::vector<std::string> files;
		GLuint handle;
		GLenum target, format, type;
		GLint levels, level, internalFormat;
		Vec2u bounds;
	protected:
		void load() override;
		void glLoad(void*) override;
		void glUnload(void*) override;
	public:
		// https://www.khronos.org/opengl/wiki/GLAPI/glTexStorage3D
		//id, files, levels, target, level, internalFormat, format, type
		Array2DTexture(std::string, std::vector<std::string>, GLuint, GLuint, GLint, GLint, GLenum, GLenum);
		GLuint get();
		void bind(GLuint);
		void setWrap(GLint, GLint);
		void setFilter(GLint, GLint);
		void setParai(GLenum, GLint);
		void setParaf(GLenum, GLfloat);
	};

	struct AtlasRegion {
		friend TextureDebugRenderer;

		TextureAtlas* parent;
		sf::Sprite* sprite;
		int texIndex = -1;
		int x, y, width, height;

		sf::Sprite* createSprite();
		Vec2 getU();
		Vec2 getV();
	};

	class TextureAtlas : public Ressource {
		friend TextureDebugRenderer;

		std::unordered_map<std::string, Texture2D*> textures;
		std::unordered_map<std::string, AtlasRegion*> regions;
	protected:
		void load() override;
		void glLoad(void*) override;
		void glUnload(void*) override;
	public:
		TextureAtlas(std::string);
		AtlasRegion* getRegion(std::string);
		static TextureAtlas* get(std::string);
	};

	class Model : public Ressource {

		ModelData* model;

	protected:
		void load() override;
		void glLoad(void*) override;
		void glUnload(void*) override;
	public:
		Model(std::string);
		ModelData* getData();
		Mat4 transform = IDENTITY;
		Vec3 position;
		void bindTransform(uint);
		void bindinvTransform(uint);
		static Model* get(std::string);
	};

	class ShaderProgram : public Ressource {
		enum Status {
			success, failed, missing
		};
		GLuint program = -1, compute = -1, vertex = -1, geom = -1, frag = -1;
		void print(std::string, Status, Status, Status, Status, Status, std::string);
		bool compile(const std::string&, const char*, const char*, const char*, const char*);
		bool loadFromMemory(const std::string&, const std::string&, const std::string&, const std::string&, const std::string&);
	protected:
		void load() override;
		void glLoad(void*) override;
		void glUnload(void*) override;
	public:
		ShaderProgram(std::string);
		bool printDebug = true;
		GLuint getHandle();		
		void bind();
		void unbind();
		static ShaderProgram* get(std::string);
	};

	class Framebuffer : public Ressource {
		GLuint handle;
		std::unordered_map<std::string, Texture2D*> textures;
	protected:
		void glLoad(void*) override;
		void glUnload(void*) override;
	public:
		Vec2u bounds;
		Framebuffer(std::string, std::unordered_map<std::string, Texture2D*>);
		void bind();
		void unbind();
		Texture2D* getTex(std::string);
		static Framebuffer* get(std::string);
	};

	class ShadowMap : public Ressource {
		Framebuffer* fb;
		std::string shadowId;
	protected:
		void glUnload(void*) override;
	public:
		ShadowMap(std::string, Framebuffer*, std::string);
		void bind();
		void unbind();
		Vec2u getBounds();
		Texture2D* getTex();
		static ShadowMap* get(std::string);
	};

	class Font : public Ressource {
		sf::Font* font;

	protected:
		void glLoad(void*) override;
		void glUnload(void*) override;

	public:
		Font(std::string);
		static Font* get(std::string);
	};

	struct Renderable {
	};

	class Renderer : public Ressource {
	public:
		Renderer(std::string);
		virtual void add(Renderable*) = 0;
		virtual void add(std::vector<Renderable*>) = 0;
		virtual void draw(View*) = 0;
	};

	struct TxDbgRenderable : public Renderable {
		enum { Texture2D, Array2DTexture, AtlasRegion, TextureAtlas };
		uint type = 0;
		float scale = 1.f;
		uint layer = 0;
		void* data;
		template<class T>
		T get();
	};

	template<class T>
	inline T TxDbgRenderable::get() {
		return reinterpret_cast<T>(data);
	}

	class TextureDebugRenderer : public Renderer {
		GLuint vao;
		ShaderProgram* shader;

		std::vector<TxDbgRenderable*> renderables;

		void draw(Texture2D*, float = 1.f);
		void draw(Array2DTexture*, uint, float = 1.f);
		void draw(AtlasRegion*, float = 1.f);
		void draw(TextureAtlas*, uint, float = 1.f);

		/*
		0: sampler2D
		1: isampler2D
		2: usampler2D
		3: sampler2DRect
		4: isampler2DRect
		5: usampler2DRect
		*/
		void draw(GLuint, GLuint, uint, uint = 0, uint = 0, uint = 0, uint = M_WIDTH, uint = M_HEIGHT);

	protected:
		void glLoad(void*) override;
		void glUnload(void*) override;
	public:
		TextureDebugRenderer();
		void add(Renderable*) override;
		void add(std::vector<Renderable*>) override;
		void draw(View*) override;
	};

	struct VSMRenderable : public Renderable {
		bool hasTex = true;
		bool isVoxel = false;
		GLuint texture, shadowTex;
		GLuint* texVox;
		Model* model;
		uint vao, offset, count, matIndex;
	};

	class ShadowRenderer : public Renderer {
		uint renderType;
		ShaderProgram* shader;
		Framebuffer* fb;
		std::vector<VSMRenderable*> renderables;
	protected:
		void glLoad(void*) override;
		void glUnload(void*) override;
	public:
		enum { VSM };
		ShadowRenderer(std::string, uint);
		void add(Renderable*) override;
		void add(std::vector<Renderable*>) override;
		void draw(View*) override;
	};

	class GaussianBlurRenderer : public Renderer {
	private:
		ShaderProgram* blurShader;
		GLuint uniformBuffer;
		GBlurData data;
	public:
		GaussianBlurRenderer(uint, float);
		~GaussianBlurRenderer();
		void blur(GLuint[2]);
	};

	class VSMRenderer : public Renderer {

		TextureDebugRenderer* debugR;
		ShadowRenderer* shadowR;
		GaussianBlurRenderer* blur;

		ShaderProgram* vsmNoTexShader;
		ShaderProgram* vsmShader;

		std::vector<VSMRenderable*> renderables;
		std::vector<VSMRenderable*> rendNoTex;
		std::vector<VSMRenderable*> voxelRend;

		void drawVSM(std::vector<VSMRenderable*>);
		void drawVSMNoTex(std::vector<VSMRenderable*>);
		void drawVoxel(std::vector<VSMRenderable*>);

	public:
		VSMRenderer(std::string);
		void add(Renderable*) override;
		void add(std::vector<Renderable*>);
		void draw(View*) override;
	};

	struct VoxelRenderable : Renderable {
		GLuint vao;
		GLuint heightmap;
	};

	class VoxelBackGroundRenderer : public Renderer {
		ShaderProgram* shader;
		GLuint bSizeBuffer[2];
		uint* bSizeBufferPntr[2];
		float* bSizeBufferPntrf[2];
		uint bSizeIndex = 0;
		std::vector<VoxelRenderable*> renderables;
	protected:
		void glLoad(void*) override;
		void glUnload(void*) override;
	public:
		uint VOXELS = 102;
		VoxelBackGroundRenderer(std::string);
		void add(Renderable*) override;
		void draw(View*) override;
	};
}
