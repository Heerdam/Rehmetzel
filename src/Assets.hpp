#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	using namespace Heerbann;

	enum Type {
		byteArray, image, texture2D, texture2DArray, 
		font, level, atlas, shader, model, renderer,
		shadowMap, framebuffer, ssbo, heightmap
	};

	struct Ressource {

		friend AssetManager;

	protected:
		std::atomic<bool> isLoaded = false;

		uint dataSize;
		void* data;

		virtual void load() {};
		virtual void unload() {};
		virtual bool glLoad(void*) { return true; };
		virtual bool glUnload(void*) { return true; };

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
		bool glLoad(void*) override;
		bool glUnload(void*) override;
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
		bool glLoad(void*) override;
		bool glUnload(void*) override;
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
		bool glLoad(void*) override;
		bool glUnload(void*) override;
	public:
		TextureAtlas(std::string);
		AtlasRegion* getRegion(std::string);
		static TextureAtlas* get(std::string);
	};

	class SSBO : public Ressource {
		GLuint handle;
		GLbitfield flags;
		uint lastBindTarget;
		void* pntr;
	protected:		
		bool glLoad(void*) override;
		bool glUnload(void*) override;
	public:
		/*
		https://www.khronos.org/opengl/wiki/GLAPI/glBufferStorage
		GL_DYNAMIC_STORAGE_BIT, GL_MAP_READ_BIT, GL_MAP_WRITE_BIT, GL_MAP_PERSISTENT_BIT,
		GL_MAP_COHERENT_BIT, GL_CLIENT_STORAGE_BIT
		*/
		SSBO(std::string, uint, void*, GLbitfield);
		void bind(uint);
		void bindAs(uint, uint);
		void unbind();
		/*
		https://www.khronos.org/opengl/wiki/GLAPI/glMapBufferRange
		!offset and length in byte!
		GL_MAP_READ_BIT, GL_MAP_WRITE_BIT, GL_MAP_INVALIDATE_RANGE_BIT,
		GL_MAP_INVALIDATE_BUFFER_BIT, GL_MAP_FLUSH_EXPLICIT_BIT,
		GL_MAP_UNSYNCHRONIZED_BIT, GL_MAP_PERSISTENT_BIT,
		GL_MAP_COHERENT_BIT
		*/
		void* map(uint, uint, GLbitfield);
		void unmap();		
		static SSBO* get(std::string);

		template<class T>
		T inline getPtr() {
			return reinterpret_cast<T>(pntr);
		};

	};

	class FlipFlopSSBO : public Ressource {
		GLbitfield flags;
		uint index = 0, size;
		std::vector<SSBO*> buffers;
		std::vector<void*> pointers;
	protected:
		bool glLoad(void*) override;
		bool glUnload(void*) override;
	public:
		FlipFlopSSBO(std::string, bool, uint, uint, GLbitfield, GLbitfield);
		void flip();
		void bind(uint);
		void bindAs(uint, uint);
		void unbind();
		template<class T>
		T inline getPtr() {
			return reinterpret_cast<T>(pointers[index]);
		};
	};

	class Model : public Ressource {
		ModelData* model;
		bool modelDataLoaded = false;
	protected:
		void load() override;
		bool glLoad(void*) override;
		bool glUnload(void*) override;
	public:
		Model(std::string);
		ModelData* getData();
		Texture2D* texture;
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
		bool glLoad(void*) override;
		bool glUnload(void*) override;
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
		bool glLoad(void*) override;
		bool glUnload(void*) override;
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
		std::string colorId, depthId;
	protected:
		bool glUnload(void*) override;
	public:
		ShadowMap(std::string, Framebuffer*, std::string, std::string);
		void bind();
		void unbind();
		Vec2u getBounds();
		Texture2D* getTex();
		Texture2D* getDepth();
		static ShadowMap* get(std::string);
	};

	class Font : public Ressource {
		sf::Font* font;

	protected:
		bool glLoad(void*) override;
		bool glUnload(void*) override;

	public:
		Font(std::string);
		static Font* get(std::string);
	};

	class HeightMap : public Ressource {
		FlipFlopSSBO* buffer;
		std::forward_list<std::tuple<bool, uint, ushort>*> changeList;
		uint width, height;
	protected:
		HeightMap(std::string, uint, uint);
		void load() override;
		bool glLoad(void*) override;
		bool glUnload(void*) override;
	public:
		void bind(uint);
		void unbind();
		void changeEntry(uint, uint, ushort);
	};

}
