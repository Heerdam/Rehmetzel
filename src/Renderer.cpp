
#include "Renderer.hpp"
#include "Assets.hpp"
#include "CameraUtils.hpp"
#include "G3D.hpp"
#include "Gdx.hpp"

using namespace Heerbann;

void TextureDebugRenderer::load() {
	shader = new ShaderProgram("assets/shader/fbo_debug_shader");
}

bool TextureDebugRenderer::glLoad(void*) {

	if (!buffersLoaded) {
		buffersLoaded = true;
		const float vertices[] = {
			1.f, 1.f, 0.f,
			1.f, 1.f,

			1.f, -1.f, 0.f,
			1.f, 0.f,

			-1.f, -1.f, 0.f,
			0.f, 0.f,

			-1.f, 1.f, 0.f,
			0.f, 1.f
		};

		const unsigned int indices[] = {
			3, 1, 0,
			3, 2, 1
		};

		GLuint index, vertex;

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &index);
		glGenBuffers(1, &vertex);

		glBindVertexArray(vao);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, vertex);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0); //a_Pos
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

		glEnableVertexAttribArray(1); //a_uv
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		GLError("TextureDebugRenderer::glLoad");
	}

	if (!shader->loaded)
		return false;

	isLoaded = true;
	return true;
}

bool TextureDebugRenderer::glUnload(void*) {
	//delete shader;
	glDeleteVertexArrays(1, &vao);
	GLError("TextureDebugRenderer::glUnload");
	return true;
}

TextureDebugRenderer::TextureDebugRenderer() : Renderer("TextureDebugRenderer") {}

void TextureDebugRenderer::add(Renderable* _renderable) {
	renderables.emplace_back(_renderable);
}

void TextureDebugRenderer::add(std::vector<Renderable*> _renderables) {
	renderables.emplace_back(_renderables.begin(), _renderables.end());
}

void TextureDebugRenderer::draw(View* _view) {
	if (!isLoaded) {
		renderables.clear();
		return;
	}
	for (Renderable* r : renderables) {
		TxDbgRenderable* rend = reinterpret_cast<TxDbgRenderable*>(r);
		if (rend->data == nullptr) continue;
		if (!reinterpret_cast<Ressource*>(rend->data)->loaded()) continue;
		switch (rend->type) {
		case TxDbgRenderable::Texture2D:
			draw(rend->get<Texture2D*>(), rend->scale);
			break;
		case TxDbgRenderable::Array2DTexture:
			draw(rend->get<Array2DTexture*>(), rend->layer, rend->scale);
			break;
		case TxDbgRenderable::AtlasRegion:
			draw(rend->get<AtlasRegion*>(), rend->scale);
			break;
		case TxDbgRenderable::TextureAtlas:
			draw(rend->get<TextureAtlas*>(), rend->layer, rend->scale);
			break;
		}
	}
	renderables.clear();
}

void TextureDebugRenderer::draw(Texture2D* _tex, float _scale) {
	uint type = 0;
	if (GL_R8I || GL_R16I || GL_R32I || GL_RG8I || GL_RG16I || GL_RG32I || GL_RGB8I ||
		GL_RGB16I || GL_RGB32I || GL_RGBA8I || GL_RGBA16I || GL_RGBA32I)
		type = 1;
	else if (GL_R8UI || GL_R16UI || GL_R32UI || GL_RG8UI || GL_RG16UI || GL_RG32UI ||
		GL_RGB8UI || GL_RGB16UI || GL_RGB32UI || GL_RGBA8UI || GL_RGBA16UI || GL_RGBA32UI || GL_RGB10_A2UI)
		type = 2;
	else type = 0;

	uint state = 0;

	switch (_tex->type) {
	case GL_TEXTURE_2D:
		state = type;
		break;
	case GL_TEXTURE_RECTANGLE:
		state = type + 3;
		break;
	}

	draw(_tex->handle, _tex->type, state, _tex->bounds.x, _tex->bounds.y, 0, _tex->bounds.x * _scale, _tex->bounds.y * _scale);
}

void TextureDebugRenderer::draw(Array2DTexture* _tex, uint _index, float _scale) {
	uint type = 0;
	if (GL_R8I || GL_R16I || GL_R32I || GL_RG8I || GL_RG16I || GL_RG32I || GL_RGB8I ||
		GL_RGB16I || GL_RGB32I || GL_RGBA8I || GL_RGBA16I || GL_RGBA32I)
		type = 1;
	else if (GL_R8UI || GL_R16UI || GL_R32UI || GL_RG8UI || GL_RG16UI || GL_RG32UI ||
		GL_RGB8UI || GL_RGB16UI || GL_RGB32UI || GL_RGBA8UI || GL_RGBA16UI || GL_RGBA32UI || GL_RGB10_A2UI)
		type = 2;
	else type = 0;

	uint state = 0;

	switch (_tex->type) {
	case GL_TEXTURE_2D:
		state = type;
		break;
	case GL_TEXTURE_RECTANGLE:
		state = type + 3;
		break;
	}

	draw(_tex->handle, _tex->type, state, _tex->bounds.x, _tex->bounds.y, _index, _tex->bounds.x * _scale, _tex->bounds.y * _scale);
}

void TextureDebugRenderer::draw(AtlasRegion* _region, float) {
	//TODO
}

void TextureDebugRenderer::draw(TextureAtlas* _atlas, uint _index, float) {
	//TODO
}

void TextureDebugRenderer::draw(GLuint _texture, GLuint _type, uint _state, uint _width, uint _height, uint _index, uint _viewportW, uint _viewportH) {
	Vec2u oldSize(M_WIDTH, M_HEIGHT);

	glViewport(0, 0, _viewportW, _viewportH);

	shader->bind();

	glActiveTexture(GL_TEXTURE0 + _state);
	glBindTexture(_type, _texture);

	glUniform1ui(3, _state);
	glUniform2ui(4, _width, _height);
	glUniform1ui(5, _index);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindTexture(_type, 0);
	glBindVertexArray(0);

	shader->unbind();

	glViewport(0, 0, oldSize.x, oldSize.y);

	GLError("TextureDebugRenderer::draw");
}

Renderer::Renderer(std::string _id) : Ressource(_id, Type::renderer) {}

void ShadowRenderer::load() {
	switch (renderType) {
	case ShadowRenderer::VSM:
		shader = new ShaderProgram("assets/shader/simple forward/sb_vsm");
		std::unordered_map <std::string, Texture2D*> textures;
		textures["depth"] = new Texture2D(id + "_fb_depth", GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
		textures["color"] = new Texture2D(id + "_fb_color", GL_TEXTURE_2D, 0, GL_RG32F, GL_RG, GL_FLOAT);
		fb = new Framebuffer("shadowFrameBuffer", textures);
		break;
	}
}

bool ShadowRenderer::glLoad(void*) {
	if (!shader->loaded() || !fb->loaded()) return false;
	isLoaded = true;
	return true;
}

bool ShadowRenderer::glUnload(void*) {
	delete fb;
	delete shader;
	return true;
}

ShadowRenderer::ShadowRenderer(std::string _id, uint _renderType) : Renderer(_id), renderType(_renderType) {}

void ShadowRenderer::add(Renderable* _renderable) {
	renderables.emplace_back(reinterpret_cast<ShadowRenderable*>(_renderable));
}

void ShadowRenderer::add(std::vector<Renderable*> _renderables) {
	for (auto r : _renderables)
		add(r);
}

void ShadowRenderer::draw(View* _view) {
	shader->bind();
	for (auto r : renderables) {
		r->light->shadowMap->bind();
		for (auto p : r->models) {
			Model* m = p.second;
			m->bindTransform(2);
			r->light->bindLightTransform(1, m->position, 1500.f, 500.f);//TODO distance for dir light?

			auto& dc = p.first;
			glBindVertexArray(dc.vao);
			glDrawElements(GL_TRIANGLES, dc.count, GL_UNSIGNED_INT, (void*)(dc.offset * sizeof(uint)));
			glBindVertexArray(0);
		}
		r->light->shadowMap->unbind();
	}
	shader->unbind();

	GLError("TestWorldLevel::draw::" + id);
}

void VoxelBackGroundRenderer::load() {
	shader = new ShaderProgram("assets/shader/voxel/shader_voxel_builder");
	buffer = new FlipFlopSSBO(id + "ssbo", 2, 7 * sizeof(uint),
		GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT,
		GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
}

bool VoxelBackGroundRenderer::glLoad(void*) {
	if (!shader->loaded() || !buffer->loaded()) return false;
	isLoaded = true;
	return true;
}

bool VoxelBackGroundRenderer::glUnload(void*) {
	delete shader;
	delete buffer;
	return true;
}

VoxelBackGroundRenderer::VoxelBackGroundRenderer(std::string _id) : Renderer(_id) {}

void VoxelBackGroundRenderer::add(Renderable* _renderable) {
	renderables.emplace_back(reinterpret_cast<VoxelRenderable*>(_renderable));
}

void VoxelBackGroundRenderer::draw(View* _view) {
	//TODO
	BoundingBox* aabb = _view->getCamera()->frustum->toAABB(_view->getCamera());
	Vec3u num_groups(VOXELS - 2, VOXELS - 2, 1);

	auto pntr = buffer->getPtr<uint*>();
	pntr[0] = 0;

	pntr[1] = VOXELS - 2;
	pntr[2] = VOXELS - 2;

	pntr[3] = 0;
	pntr[4] = 0;

	pntr[5] = 50;
	pntr[6] = 5;

	shader->bind();
	for (auto r : renderables) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, r->vao);
		r->heightmap->bind(1);
		glDispatchCompute(num_groups.x, num_groups.y, num_groups.z);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
	}
	buffer->flip();
	shader->unbind();
	renderables.clear();
	GLError("VoxelBackGroundRenderer::draw");
}

void Heerbann::VSMLightRenderer::load() {
}

bool Heerbann::VSMLightRenderer::glLoad(void *) {
	return false;
}

bool VSMLightRenderer::glUnload(void*) {
	delete shader;
	return true;
}

VSMLightRenderer::VSMLightRenderer(std::string _id) : Renderer(_id) {
	shader = new ShaderProgram("shader/vsm/shader_vsm_s2_light");
}

void VSMLightRenderer::add(Renderable* _renderable) {
	renderables.emplace_back(reinterpret_cast<VSMLightRenderable*>(_renderable));
}

void VSMLightRenderer::add(std::vector<Renderable*> _renderables) {
	for (auto r : _renderables)
		add(r);
}

void VSMLightRenderer::draw(View* _view) {
	if (!loaded()) {
		renderables.clear();
		return;
	}
	shader->bind();
	for (auto r : renderables) {

		r->model->bindTransform(3);
		r->model->bindinvTransform(4);

		_view->bindCombined(5);
		_view->bindPosition(6);

		r->lightBuffer->bind(1);

		if (r->texture != nullptr)
			r->texture->bind(GL_TEXTURE0);

		r->matBuffer->bind(2);

		glUniform1ui(7, r->matIndex);
		glUniform1ui(8, r->texture == nullptr ? 0 : 1);

		glBindVertexArray(r->drawC.vao);
		glDrawElements(GL_TRIANGLES, r->drawC.offset, GL_UNSIGNED_INT, (void*)(r->drawC.count * sizeof(uint)));
		glBindVertexArray(0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	}
	shader->unbind();
	renderables.clear();
	GLError("VSMLightRenderer::draw");
}

void VSMShadowRenderer::load() {
	shader = new ShaderProgram("shader/vsm/shader_vsm_s3_shadow");
}

bool VSMShadowRenderer::glLoad(void*) {
	if (!shader->loaded()) return false;
	isLoaded = true;
	return true;
}

bool VSMShadowRenderer::glUnload(void*) {
	delete shader;
	return true;
}

VSMShadowRenderer::VSMShadowRenderer(std::string _id) : Renderer(_id) {}

void VSMShadowRenderer::add(Renderable* _renderable) {
	renderables.emplace_back(reinterpret_cast<VSMShadownRenderable*>(_renderable));
}

void VSMShadowRenderer::add(std::vector<Renderable*> _renderables) {
	for (auto r : _renderables)
		add(r);
}

void VSMShadowRenderer::draw(View* _view) {
	shader->bind();
	_view->bindCombined(2);
	for (auto r : renderables) {
		r->model->bindTransform(2);
		for (auto l : r->lights) {
			l->bindLightTransform(4, r->model->position, 1000.f, 500.f); //TODO
			l->shadowMap->getDepth()->bind(1);
			l->shadowMap->getTex()->bind(2);
			glBindVertexArray(vao);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}
	shader->unbind();
	GLError("VSMShadowRenderer::draw::" + id);
}

void VSMRenderer::load() {
	debugR = new TextureDebugRenderer();
	//blur = new GaussianBlur(128, 8);

	shadowMapR = new ShadowRenderer(id + "_shadowMapR", ShadowRenderer::VSM);
	lightR = new VSMLightRenderer(id + "_lightR");
	shadowR = new VSMShadowRenderer(id + "_shadowR");
}

bool VSMRenderer::glLoad(void*) {
	if (!debugR->loaded() || !shadowMapR->loaded() || !lightR->loaded() || !shadowR->loaded())
		return false;
	isLoaded = true;
	return true;
}

bool VSMRenderer::glUnload(void*) {
	delete debugR;
	delete shadowMapR;
	delete lightR;
	delete shadowR;
	return true;
}

VSMRenderer::VSMRenderer(std::string _id) : Renderer(_id) {}

void VSMRenderer::add(Renderable* _renderable) {
	renderables.emplace_back(reinterpret_cast<VSMRenderable*>(_renderable));
}

void VSMRenderer::add(std::vector<Renderable*> _renderables) {
	for (auto r : _renderables)
		add(r);
}

void VSMRenderer::draw(View* _view) {


	auto lights = M_Env->queryLights(_view);

	//shadowMapR
	std::vector<Renderable*> shadowRenderables(lights.size());
	std::vector<std::pair<DrawCall, Model*>> shadowDrawCalls(renderables.size);
	for (auto r : renderables) {
		shadowDrawCalls.emplace_back(std::make_pair(r->drawC, r->model));
	}

	for (auto l : lights) {
		ShadowRenderable* out = new ShadowRenderable();
		out->light = l;
		out->models = shadowDrawCalls;
		shadowRenderables.emplace_back(out);
	}
	shadowMapR->add(shadowRenderables);

	//lightR
	std::vector<Renderable*> VSMLightRenderables(renderables.size);
	for (auto r : renderables) {
		VSMLightRenderable* out = new VSMLightRenderable();
		out->drawC = r->drawC;
		out->model = r->model;
		out->matBuffer = r->model->getData()->matBuffer;
		out->texture = r->tex;
		out->matIndex = r->matIndex;
		VSMLightRenderables.emplace_back(out);
	}
	lightR->add(VSMLightRenderables);

	//shadowR
	std::vector<Renderable*> shadowRenderables(renderables.size);
	for (auto r : renderables) {
		VSMShadowRenderable* out = new VSMShadowRenderable();
		out->lights = lights;
		out->model = r->model;
		shadowRenderables.emplace_back(out);
	}
	shadowR->add(shadowRenderables);

	//draw
	shadowMapR->draw(_view);
	lightR->draw(_view);
	shadowR->draw(_view);

	for (auto r : shadowRenderables)
		delete r;

	for (auto r : VSMLightRenderables)
		delete r;

	renderables.clear();
}