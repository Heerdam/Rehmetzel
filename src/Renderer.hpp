#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

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
		bool buffersLoaded = false;

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
		void load() override;
		bool glLoad(void*) override;
		bool glUnload(void*) override;
	public:
		TextureDebugRenderer();
		void add(Renderable*) override;
		void add(std::vector<Renderable*>) override;
		void draw(View*) override;
	};

	struct ShadowRenderable : Renderable {
		Light* light;
		std::vector<std::pair<DrawCall, Model*>> models;
	};

	class ShadowRenderer : public Renderer {
		uint renderType;
		ShaderProgram* shader;
		Framebuffer* fb;
		std::vector<ShadowRenderable*> renderables;
	protected:
		void load() override;
		bool glLoad(void*) override;
		bool glUnload(void*) override;
	public:
		enum { VSM };
		ShadowRenderer(std::string, uint);
		void add(Renderable*) override;
		void add(std::vector<Renderable*>) override;
		void draw(View*) override;
	};

	struct GaussianBlurRenderable : Renderable {

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

	struct VoxelRenderable : Renderable {
		Texture2D* heightmap;
		GLuint vao;
	};

	class VoxelBackGroundRenderer : public Renderer {
		ShaderProgram* shader;
		FlipFlopSSBO* buffer;
		std::vector<VoxelRenderable*> renderables;
	protected:
		void load() override;
		bool glLoad(void*) override;
		bool glUnload(void*) override;
	public:
		uint VOXELS = 102;
		VoxelBackGroundRenderer(std::string);
		void add(Renderable*) override;
		void draw(View*) override;
	};

	struct VSMLightRenderable : public Renderable {
		DrawCall drawC;

		Model* model;
		SSBO* lightBuffer;
		SSBO* matBuffer;
		Texture2D* texture;
		uint matIndex;
	};

	class VSMLightRenderer : public Renderer {
		ShaderProgram* shader;
		std::vector<VSMLightRenderable*> renderables;
	protected:
		void load() override;
		bool glLoad(void*) override;
		bool glUnload(void*) override;
	public:
		VSMLightRenderer(std::string);
		void add(Renderable*) override;
		void add(std::vector<Renderable*>) override;
		void draw(View*) override;
	};

	struct VSMShadowRenderable : Renderable {
		Model* model;
		std::vector<Light*> lights;
	};

	class VSMShadowRenderer : public Renderer {
		GLuint vao;
		ShaderProgram* shader;
		std::vector<VSMShadowRenderable*> renderables;
	protected:
		void load() override;
		bool glLoad(void*) override;
		bool glUnload(void*) override;
	public:
		VSMShadowRenderer(std::string);
		void add(Renderable*) override;
		void add(std::vector<Renderable*>) override;
		void draw(View*) override;
	};

	struct VSMRenderable : public Renderable {
		Model* model;
		uint matIndex;
		Texture2D* tex;
		DrawCall drawC;
	};

	class VSMRenderer : public Renderer {
		std::vector<VSMRenderable*> renderables;
		//utility
		TextureDebugRenderer* debugR;
		//GaussianBlurRenderer* blur;

		//renders shadowmaps
		ShadowRenderer* shadowMapR;
		//shaded geometry to fbo
		VSMLightRenderer* lightR;
		//draws shadows to output
		VSMShadowRenderer* shadowR;
	protected:
		void load() override;
		bool glLoad(void*) override;
		bool glUnload(void*) override;
	public:
		VSMRenderer(std::string);
		void add(Renderable*) override;
		void add(std::vector<Renderable*>) override;
		void draw(View*) override;
	};

	

}