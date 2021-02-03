#include "..\..\Rehmetzel_v2\include\Rehmetzel\GL\Texture.hpp"

#include <fstream>

#include "Assets.hpp"
#include "Level.h"
#include "TextUtil.hpp"
#include "Utils.hpp"
#include "TimeLog.hpp"
#include "Gdx.hpp"
#include "G3D.hpp"
#include "CameraUtils.hpp"
#include "Math.hpp"

using namespace Heerbann;

Ressource::Ressource(std::string _id, Type _type) : type(_type), id(_id){
	M_Asset->loadFromDisk(_id, this);
}

Ressource::~Ressource() {
	M_Asset->unload(this);
}

void AssetManager::loadFromDisk(std::string _id, Ressource* _res) {
	assets[_id] = _res;
	#pragma omp parallel
	{
		_res->load();
		M_Main->addJob(_res->glLoad, _res);
	}
}

void AssetManager::unload(Ressource* _res) {
	assets.erase(_res->id);
	#pragma omp parallel
	{
		_res->unload();
		M_Main->addJob(_res->glUnload, _res);
	}
}

bool AssetManager::exists(std::string _id) {
	return assets.count(_id) > 0;
}

Image::Image(std::string _id) : Ressource(_id, Type::image) {}

void Image::load() {
	std::ifstream ifs(id, std::ios::binary);
	if (!ifs.good()) throw new std::exception((std::string("can't open file [") + id + std::string("]")).data());
	ifs.seekg(0, std::ios::end);
	dataSize = (int)ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	char* data = new char[dataSize];
	ifs.read(data, dataSize);
	ifs.close();
	image = new sf::Image();
	image->loadFromMemory(data, dataSize);
	isLoaded = true;
}

void Image::unload() {
	delete image;
	image = nullptr;
}

sf::Image* Image::get() {
	return image;
}

void Image::finish() {
	if (isLoaded) return;
	using namespace std::chrono_literals;
	auto timestamp = TIMESTAMP;
	while (!isLoaded) {
		std::this_thread::sleep_for(0.1ms);
		unsigned long long elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(TIMESTAMP.time - timestamp.time).count();
		assert(elapsedTime < 4 && "time out!");
	}
}

Image* Image::get(std::string _id) {
	return M_Asset->get<Image*>(_id);
}

Texture2D::Texture2D(std::string _id, GLuint _target, GLint _level, GLint _internalFormat, GLenum _format, GLenum _type) : 
	Ressource(_id, Type::texture2D), target(_target), level(_level), 
	internalFormat(_internalFormat), format(_format), type(_type) {}

void Texture2D::load() {
	std::ifstream ifs(id, std::ios::binary);
	if (ifs.good()) {
		ifs.seekg(0, std::ios::end);
		dataSize = (int)ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		char* dataC = new char[dataSize];
		ifs.read(dataC, dataSize);		
		sf::Image* img = new sf::Image();
		data = img;
		img->loadFromMemory(dataC, dataSize);
		bounds = Vec2u(img->getSize().x, img->getSize().y);
		delete dataC;
	}
	ifs.close();
}

bool Texture2D::glLoad(void*) {
	sf::Image* img = reinterpret_cast<sf::Image*>(data);
	glGenTextures(1, &handle);
	glBindTexture(target, handle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(target, level, internalFormat, img->getSize().x, img->getSize().y, 0, format, type, img->getPixelsPtr());
	glBindTexture(target, 0);
	delete data;
	data = nullptr;
	isLoaded = true;
	GLError("Texture2D::glLoad");
	return true;
}

bool Texture2D::glUnload(void*) {
	glDeleteTextures(1, &handle);
	return true;
}

GLuint Texture2D::get() {
	return handle;
}

void Texture2D::bind(GLuint _binding) {
	glActiveTexture(GL_TEXTURE0 + _binding);
	glBindTexture(target, handle);
	GLError("Texture2D::bind");
}

void Texture2D::setWrap(GLint _s, GLint _t) {
	glBindTexture(target, handle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _t);
	glBindTexture(target, 0);
	GLError("Texture2D::setWrap");
}

void Texture2D::setFilter(GLint _min, GLint _mag) {
	glBindTexture(target, handle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _min);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _mag);
	glBindTexture(target, 0);
	GLError("Texture2D::setFilter");
}

void Texture2D::setParai(GLenum _pname, GLint _param) {
	glBindTexture(target, handle);
	glTexParameteri(GL_TEXTURE_2D, _pname, _param);
	glBindTexture(target, 0);
	GLError("Texture2D::setParai");
}

void Texture2D::setParaf(GLenum _pname, GLfloat _param) {
	glBindTexture(target, handle);
	glTexParameterf(GL_TEXTURE_2D, _pname, _param);
	glBindTexture(target, 0);
	GLError("Texture2D::setParaf");
}

Texture2D* Texture2D::get(std::string _id) {
	return M_Asset->get<Texture2D*>(_id);
}

Array2DTexture::Array2DTexture(std::string _id, std::vector<std::string> _files, GLuint _levels, GLuint _target, 
	GLint _level, GLint _internalFormat, GLenum _format, GLenum _type) :
	Ressource(_id, Type::texture2DArray), files(_files), levels(_levels), target(_target), level(_level),
	internalFormat(_internalFormat), format(_format), type(_type){}

void Array2DTexture::load() {
	dataSize = files.size();
	Image** imgs = new Image*[dataSize];
	for (uint i = 0; i < dataSize; ++i) {
		imgs[i] = new Image(files[i]);
	}
	for (uint i = 0; i < dataSize; ++i) {
		imgs[i]->finish();
	}
}

bool Array2DTexture::glLoad(void*) {
	assert(dataSize != 0 && "datasize == 0, no images loaded");
	assert(data != nullptr && "data is null");
	glGenTextures(GL_TEXTURE_2D_ARRAY, &handle);
	glBindTexture(GL_TEXTURE_2D_ARRAY, handle);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GLError("Array2DTexture::glLoad::glGenTextures");
	
	Image** imgs = reinterpret_cast<Image**>(data);
	auto image = imgs[0];
	Vec2u bounds = Vec2u(image->get()->getSize().x, image->get()->getSize().y);
	Vec2u size(image->get()->getSize().x, image->get()->getSize().y);
	//reserve storage
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, levels, internalFormat, size.x, size.y, dataSize);
	GLError("Array2DTexture::glLoad::glTexStorage3D");
	//add files
	for (uint i = 0; i < dataSize; ++i) {
		Image* im = imgs[i];
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, size.x, size.y, 1, format, type, image->get()->getPixelsPtr());
		GLError("Array2DTexture::glLoad::glTexSubImage3D");
	}
	//glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	//GLError("Array2DTexture::glLoad::glGenerateMipmap");
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	delete data;
	data = nullptr;
	isLoaded = true;
	return true;
}

bool Array2DTexture::glUnload(void*) {
	glDeleteTextures(1, &handle);
	GLError("Array2DTexture::glUnload");
	return true;
}

GLuint Array2DTexture::get() {
	return handle;
}

void Array2DTexture::bind(GLuint) {
	glBindTexture(GL_TEXTURE_2D_ARRAY, handle);
	GLError("Array2DTexture::bind");
}

void Array2DTexture::setWrap(GLint _s, GLint _t) {
	glBindTexture(target, handle);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, _s);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, _t);
	glBindTexture(target, 0);
	GLError("Array2DTexture::setWrap");
}

void Array2DTexture::setFilter(GLint _min, GLint _mag) {
	glBindTexture(target, handle);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, _min);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, _mag);
	glBindTexture(target, 0);
	GLError("Array2DTexture::setFilter");
}

void Array2DTexture::setParai(GLenum _pname, GLint _param) {
	glBindTexture(target, handle);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, _pname, _param);
	glBindTexture(target, 0);
	GLError("Array2DTexture::setParai");
}

void Array2DTexture::setParaf(GLenum _pname, GLfloat _param) {
	glBindTexture(target, handle);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, _pname, _param);
	glBindTexture(target, 0);
	GLError("Array2DTexture::setParaf");
}

sf::Sprite* AtlasRegion::createSprite() {
	if (sprite != nullptr) return sprite;
	sprite = new sf::Sprite();
	sprite->setTextureRect(sf::IntRect(sf::Vector2i(x, y), sf::Vector2i(width, height)));
	return sprite;
}

Vec2 AtlasRegion::getU() {
	return Vec2();
	//return Vec2(1.f / (float)parent->tex[texIndex]->getSize().x * (float)x, 1.f / (float)parent->tex[texIndex]->getSize().x * (float)(x + width));
}

Vec2 AtlasRegion::getV() {
	return Vec2();
	//return Vec2(1.f / (float)parent->tex[texIndex]->getSize().y * (float)y, 1.f / (float)parent->tex[texIndex]->getSize().y * (float)(y + height));
}

TextureAtlas::TextureAtlas(std::string _id) : Ressource(_id, Type::atlas) {}

AtlasRegion* TextureAtlas::getRegion(std::string _id) {
	if (regions.count(_id) == 0) throw new std::exception((std::string("region does not exist [") + _id + std::string("]")).c_str());
	return regions[_id];
}

TextureAtlas* TextureAtlas::get(std::string _id) {
	return M_Asset->get<TextureAtlas*>(_id);
}

void Heerbann::TextureAtlas::load() {
	std::ifstream file(id + std::string(".atlas"));

	int lineNr = 1;
	int imgNr = 0;

	bool newImg = false;

	AtlasRegion* cR = nullptr;
	int regionNr = 1;

	for (std::string line; getline(file, line); ++lineNr) {

		//first 5 lines
		switch (lineNr) {
		case 1:
		{
			std::string fileId = id.substr(0, id.find_last_of("/") + 1) + line;
			textures[fileId] = new Texture2D(fileId, GL_TEXTURE_2D, 0, GL_RGBA16F, GL_RGBA, GL_FLOAT);
			continue;
		}
		break;
		case 2: continue;
		case 3: continue;
		case 4: continue;
		case 5: continue;
		}

		//new img
		if (line.empty()) {
			++imgNr;
			lineNr = 0;
			continue;
		}

		//atlasregion
		switch (regionNr) {
		case 1: //name
		{
			++regionNr;
			cR = new AtlasRegion();
			cR->parent = this;
			cR->texIndex = imgNr;
			regions[line] = cR;
			//atlas->regionList.emplace_back(cR);
			continue;
		}
		break;
		case 2:
			++regionNr;
			continue;
		case 3: //xy
		{
			++regionNr;
			int pos1 = static_cast<int>(line.find_first_of(":") + 2);
			int pos2 = static_cast<uint>(line.find_first_of(","));
			int pos3 = static_cast<uint>(line.length() - (pos2 + 1));

			auto x = line.substr(pos1, pos2 - pos1);
			auto y = line.substr(pos2 + 2, pos3 + 1);

			cR->x = std::stoi(x);
			cR->y = std::stoi(y);
		}
		break;
		case 4: //size
		{
			++regionNr;
			int pos1 = static_cast<uint>(line.find_first_of(":") + 2);
			int pos2 = static_cast<uint>(line.find_first_of(","));
			int pos3 = static_cast<uint>(line.length() - (pos2 + 1));

			auto w = line.substr(pos1, pos2 - pos1);
			auto h = line.substr(pos2 + 2, pos3 + 1);

			cR->width = std::stoi(w);
			cR->height = std::stoi(h);
		}
		break;
		case 5:
			++regionNr;
			continue;
		case 6:
			++regionNr;
			continue;
		case 7:
			regionNr = 1;
			continue;
		}
	}

	//atlas->tex.resize(atlas->files.size());
	//atlas->img.resize(atlas->files.size());
	file.close();
}

bool TextureAtlas::glLoad(void*) {
	//TODO
	return true;
}

bool TextureAtlas::glUnload(void*) {
	//TODO
	return true;
}

SSBO::SSBO(std::string _id, uint _size, void* _data, GLbitfield _flags) : Ressource(_id, Type::ssbo) {
	data = _data;
	dataSize = _size;
	flags = _flags;
}

bool SSBO::glLoad(void*) {
	glGenBuffers(1, &handle);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, dataSize, data, flags);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	GLError("SSBO::glLoad::" + id);
	return true;
}

bool SSBO::glUnload(void*) {
	unmap();
	glDeleteBuffers(1, &handle);
	GLError("SSBO::glUnload::" + id);
	return true;
}

void SSBO::bind(uint _binding) {
	bindAs(GL_SHADER_STORAGE_BUFFER, _binding);
}

void SSBO::bindAs(uint _target, uint _binding) {
	glBindBufferBase(lastBindTarget = _target, _binding, handle);
	GLError("SSBO::bindAs::" + id);
}

void SSBO::unbind() {
	glBindBuffer(lastBindTarget, 0);
	GLError("SSBO::unbind::" + id);
}

void* SSBO::map(uint _offset, uint _length, GLbitfield _flags) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
	pntr = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, _offset, _length, _flags);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	GLError("SSBO::map::" + id);
	return pntr;
}

void SSBO::unmap() {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	GLError("SSBO::unmap::" + id);
}

SSBO* SSBO::get(std::string _id) {
	return M_Asset->get<SSBO*>(_id);
}

void Model::load() {
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(id, aiProcess_ValidateDataStructure | aiProcess_FixInfacingNormals |
		aiProcess_FlipUVs);

	//vec3 pos
	//vec3 normals
	//vev2 uv
	//float index
	std::vector<float> vertexBuffer;
	std::vector<unsigned int> indexBuffer;
	//offset, size

	model = new ModelData();

	std::vector<Material> materialList;

	//Material
	materialList.resize(scene->mNumMaterials);
	for (uint i = 0; i < scene->mNumMaterials; ++i) {
		Material material;
		auto mat = scene->mMaterials[i];

		aiColor3D out;

		mat->Get(AI_MATKEY_COLOR_DIFFUSE, out);
		material.COLOR_DIFFUSE = Vec4(out.r, out.g, out.b, 1.f);

		mat->Get(AI_MATKEY_COLOR_SPECULAR, out);
		material.COLOR_SPECULAR = Vec4(out.r, out.g, out.b, 1.f);

		mat->Get(AI_MATKEY_COLOR_AMBIENT, out);
		material.COLOR_AMBIENT = Vec4(out.r, out.g, out.b, 1.f);

		mat->Get(AI_MATKEY_COLOR_EMISSIVE, out);
		material.COLOR_EMISSIVE = Vec4(out.r, out.g, out.b, 1.f);

		mat->Get(AI_MATKEY_COLOR_TRANSPARENT, out);
		material.COLOR_TRANSPARENT = Vec4(out.r, out.g, out.b, 1.f);

		mat->Get(AI_MATKEY_OPACITY, material.vals.x);

		mat->Get(AI_MATKEY_SHININESS, material.vals.y);

		mat->Get(AI_MATKEY_SHININESS_STRENGTH, material.vals.z);

		materialList[i] = material;
	}

	//Vertex & Index data
	int meshIndexOffset = 0;
	int meshVertexOffset = 0;
	//meshes

	model->meshList.resize(scene->mNumMeshes);

	model->boneCache.resize(scene->mNumMeshes);

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {

		aiMesh* mesh = scene->mMeshes[i];
		Mesh* meshOut = new Mesh();
		model->meshList[i] = meshOut;
		model->meshMap[std::string(mesh->mName.C_Str())] = meshOut;

		meshOut->vertexCount = mesh->mNumVertices;
		meshOut->vertexOffset = static_cast<uint>(vertexBuffer.size());
		meshOut->matIndex = mesh->mMaterialIndex;

		//vertex
		for (unsigned int k = 0; k < mesh->mNumVertices; ++k) {
			auto pos = mesh->mVertices[k];
			auto norm = mesh->mNormals[k];
			auto uv = mesh->mTextureCoords[0];

			//pos
			vertexBuffer.emplace_back(pos.x);
			vertexBuffer.emplace_back(pos.z);
			vertexBuffer.emplace_back(pos.y);
			//norm
			vertexBuffer.emplace_back(norm.x);
			vertexBuffer.emplace_back(norm.z);
			vertexBuffer.emplace_back(norm.y);
			//uv
			vertexBuffer.emplace_back(uv == NULL ? 0.f : uv[k].x);
			vertexBuffer.emplace_back(uv == NULL ? 0.f : uv[k].y);

			meshVertexOffset += mesh->mNumVertices;
		}

		//index
		int iCount = 0;
		for (unsigned int k = 0; k < mesh->mNumFaces; ++k) {
			auto& face = mesh->mFaces[k];
			for (unsigned int j = 0; j < face.mNumIndices; ++j) {
				indexBuffer.emplace_back(meshIndexOffset + face.mIndices[j]);
				++iCount;
			}
		}

		meshOut->indexCount = iCount;
		meshOut->indexOffset = meshIndexOffset;

		meshIndexOffset += iCount;

		model->boneCache[i].resize(mesh->mNumBones);
		for (uint k = 0; k < mesh->mNumBones; ++k) {
			auto bone = mesh->mBones[i];
			Bone* out = new Bone();
			model->boneCache[i].emplace_back(out);
			out->id = bone->mName.C_Str();
			meshOut->boneMap[out->id] = out;
			out->numWeights = bone->mNumWeights;
			out->offset = Mat4(
				bone->mOffsetMatrix.a1, bone->mOffsetMatrix.b1, bone->mOffsetMatrix.c1, bone->mOffsetMatrix.d1,
				bone->mOffsetMatrix.a2, bone->mOffsetMatrix.b2, bone->mOffsetMatrix.c2, bone->mOffsetMatrix.d2,
				bone->mOffsetMatrix.a3, bone->mOffsetMatrix.b3, bone->mOffsetMatrix.c3, bone->mOffsetMatrix.d3,
				bone->mOffsetMatrix.a4, bone->mOffsetMatrix.b4, bone->mOffsetMatrix.c4, bone->mOffsetMatrix.d4);
			out->weights.resize(bone->mNumWeights);
			for (uint j = 0; j < bone->mNumWeights; ++j)
				out->weights[j] = std::make_tuple(bone->mWeights[j].mVertexId, bone->mWeights[j].mWeight);
		}

	}

	//create node hierarchy
	std::function<mNode*(mNode*, aiNode*)> sort = [&](mNode* _parent, aiNode* _self)->mNode* {

		mNode* out = new mNode();
		if (_parent != nullptr) {
			out->parent = _parent;
			_parent->children.emplace_back(out);
		}
		out->id = _self->mName.C_Str();
		out->meshes.resize(_self->mNumMeshes);
		std::memcpy(&out->meshes[0], _self->mMeshes, _self->mNumMeshes * sizeof(uint));
		out->transform = Mat4(
			_self->mTransformation.a1, _self->mTransformation.b1, _self->mTransformation.c1, _self->mTransformation.d1,
			_self->mTransformation.a2, _self->mTransformation.b2, _self->mTransformation.c2, _self->mTransformation.d2,
			_self->mTransformation.a3, _self->mTransformation.b3, _self->mTransformation.c3, _self->mTransformation.d3,
			_self->mTransformation.a4, _self->mTransformation.b4, _self->mTransformation.c4, _self->mTransformation.d4);

		model->nodeMap[out->id] = out;
		model->nodeCache.emplace_back(out);
		if (_self->mNumChildren == 0) return out;
		for (uint i = 0; i < _self->mNumChildren; ++i)
			sort(out, _self->mChildren[i]);
		return out;
	};

	//modelOut->root = sort(nullptr, scene->mRootNode);

	//animations
	for (uint i = 0; i < scene->mNumAnimations; ++i) {
		auto an = scene->mAnimations[i];
		Animation* out = new Animation();
		out->duration = FLOAT(an->mDuration);
		out->ticksPerSecond = FLOAT(an->mTicksPerSecond);

		for (uint j = 0; j < an->mNumChannels; ++j) {
			auto chan = an->mChannels[j];
			NodeAnimation* na = new NodeAnimation();
			na->affectedNode = chan->mNodeName.C_Str();

			na->positionKeys.resize(chan->mNumPositionKeys);
			for (uint k = 0; k < chan->mNumPositionKeys; ++k) {
				auto vec = chan->mPositionKeys[k];
				na->positionKeys[k] = VectorKey{ FLOAT(vec.mTime), Vec3(vec.mValue.x, vec.mValue.y, vec.mValue.z) };
			}

			na->quatKeys.resize(chan->mNumRotationKeys);
			for (uint k = 0; k < chan->mNumRotationKeys; ++k) {
				auto quat = chan->mRotationKeys[k];
				na->quatKeys[k] = QuatKey{ FLOAT(quat.mTime), Quat(quat.mValue.x, quat.mValue.y, quat.mValue.z, quat.mValue.w) };
			}

			na->scalingKeys.resize(chan->mNumScalingKeys);
			for (uint k = 0; k < chan->mNumScalingKeys; ++k) {
				auto vec = chan->mPositionKeys[k];
				na->scalingKeys[k] = VectorKey{ FLOAT(vec.mTime), Vec3(vec.mValue.x, vec.mValue.y, vec.mValue.z) };
			}
		}

		for (uint j = 0; j < an->mNumMeshChannels; ++j) {
			auto chan = an->mMeshChannels[j];
			MeshAnimation* ma = new MeshAnimation();
			ma->affectedMesh = chan->mName.C_Str();
			ma->keys.resize(chan->mNumKeys);
			for (uint k = 0; k < chan->mNumKeys; ++k) {
				auto vec = chan->mKeys[k];
				ma->keys[k] = MeshKey{ FLOAT(vec.mTime), vec.mValue };
			}
		}
	}

	model->vertexBufferCacheSize = static_cast<uint>(vertexBuffer.size());
	model->vertexBufferCache = new float[vertexBuffer.size()];
	std::memcpy(model->vertexBufferCache, vertexBuffer.data(), vertexBuffer.size() * sizeof(float));

	model->indexBufferCacheSize = static_cast<uint>(indexBuffer.size());
	model->indexBufferCache = new unsigned int[indexBuffer.size()];
	std::memcpy(model->indexBufferCache, indexBuffer.data(), indexBuffer.size() * sizeof(unsigned int));

	model->matBuffer = new SSBO(id + "_matBuffer", sizeof(Material) * materialList.size(), materialList.data(), 0);

}

bool Model::glLoad(void*) {

	if (!modelDataLoaded) {
		modelDataLoaded = true;
		model = reinterpret_cast<ModelData*>(data);
		LOG("Loading: [Model] " + id);
		GLuint vbo;
		GLuint index;

		//create buffer
		glGenVertexArrays(1, &model->vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &index);

		glBindVertexArray(model->vao);

		//vbo
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model->vertexBufferCacheSize, model->vertexBufferCache, GL_STATIC_DRAW);

		//index
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * model->indexBufferCacheSize, model->indexBufferCache, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		if (model->vertexBufferCache != nullptr) delete model->vertexBufferCache;
		if (model->indexBufferCache != nullptr) delete model->indexBufferCache;
		if (model->animationCache != nullptr) delete model->animationCache;

		GLError("Model::glLoad");
	}
	if (!model->matBuffer->loaded()) return false;
	isLoaded = true;	
	return true;
}

bool Heerbann::Model::glUnload(void *) {
	//TODO
	return true;
}

Model::Model(std::string _id) : Ressource(_id, Type::model) {}

ModelData * Heerbann::Model::getData() {
	return model;
}

void Model::bindTransform(uint _location) {
	glUniformMatrix4fv(_location, 1, false, ToArray(transform));
	GLError("Model::bindTransform");
}

void Model::bindinvTransform(uint _location) {
	glUniformMatrix3fv(_location, 1, true, ToArray(INV(Mat3(transform))));
	GLError("Model::bindinvTransform");
}

Model* Model::get(std::string _id) {
	return M_Asset->get<Model*>(_id);
}

void ShaderProgram::print(std::string _id, ShaderProgram::Status _compComp, ShaderProgram::Status _compVert,
	ShaderProgram::Status _compGeom, ShaderProgram::Status _compFrag, ShaderProgram::Status _link, std::string _errorLog) {
	if (!printDebug) return;
	LOG("   Shader: " + std::string(_id));
	LOG("Compiling: "
		+ std::string(_compComp == Status::failed ? " X |" : _compComp == Status::success ? " S |" : " - |")
		+ std::string(_compVert == Status::failed ? " X |" : _compVert == Status::success ? " S |" : " - |")
		+ std::string(_compGeom == Status::failed ? " X |" : _compGeom == Status::success ? " S |" : " - |")
		+ std::string(_compFrag == Status::failed ? " X |" : _compFrag == Status::success ? " S |" : " - |")
		+ "\n");
	LOG("  Linking: " + std::string(_link == Status::failed ? "Failed!" : _link == Status::success ? "Success!" : " - ") + "\n");

	if (_errorLog.empty()) {
		LOG("\n");
	} else {
		LOG("\n" + std::string(_errorLog) + "\n");
	}

	App::Gdx::printOpenGlErrors(_id);
	std::cout << std::endl;
}

bool ShaderProgram::compile(const std::string& _id, const char* _compute, const char* _vertex, const char* _geom, const char* _frag) {
	Status compStatus = Status::missing;
	Status vertStatus = Status::missing;
	Status geomStatus = Status::missing;
	Status fragStatus = Status::missing;
	Status linkStatus = Status::missing;

	if (compute != -1) {
		glDeleteShader(compute);
		compute = -1;
	}
	if (vertex != -1) {
		glDeleteShader(vertex);
		vertex = -1;
	}
	if (geom != -1) {
		glDeleteShader(geom);
		geom = -1;
	}
	if (frag != -1) {
		glDeleteShader(frag);
		frag = -1;
	}
	if (program != -1) {
		glDeleteShader(program);
		program = -1;
	}

	//Compile Compute
	if (_compute != nullptr) {
		compute = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(compute, 1, &_compute, nullptr);
		glCompileShader(compute);
		GLint isCompiled = 0;
		glGetShaderiv(compute, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(compute, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> errorLog(maxLength);
			glGetShaderInfoLog(compute, maxLength, &maxLength, &errorLog[0]);
			glDeleteShader(compute);
			compStatus = Status::failed;
			print(_id, compStatus, vertStatus, geomStatus, fragStatus, linkStatus, std::string(errorLog.begin(), errorLog.end()));
			return false;
		} else compStatus = Status::success;
	}

	//Compile Vertex
	if (_vertex != nullptr) {
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &_vertex, nullptr);
		glCompileShader(vertex);
		GLint isCompiled = 0;
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> errorLog(maxLength);
			glGetShaderInfoLog(vertex, maxLength, &maxLength, &errorLog[0]);
			glDeleteShader(vertex);
			vertStatus = Status::failed;
			print(_id, compStatus, vertStatus, geomStatus, fragStatus, linkStatus, std::string(errorLog.begin(), errorLog.end()));
			return false;
		} else vertStatus = Status::success;
	}

	//Compile Geom
	if (_geom != nullptr) {
		geom = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geom, 1, &_geom, nullptr);
		glCompileShader(geom);
		GLint isCompiled = 0;
		glGetShaderiv(geom, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(geom, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> errorLog(maxLength);
			glGetShaderInfoLog(geom, maxLength, &maxLength, &errorLog[0]);
			glDeleteShader(geom);
			geomStatus = Status::failed;
			print(_id, compStatus, vertStatus, geomStatus, fragStatus, linkStatus, std::string(errorLog.begin(), errorLog.end()));
			return false;
		} else geomStatus = Status::success;
	}

	//Compile Frag
	if (_frag != nullptr) {
		frag = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(frag, 1, &_frag, nullptr);
		glCompileShader(frag);
		GLint isCompiled = 0;
		glGetShaderiv(frag, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			GLint maxLength = 0;
			glGetShaderiv(frag, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> errorLog(maxLength);
			glGetShaderInfoLog(frag, maxLength, &maxLength, &errorLog[0]);
			glDeleteShader(frag);
			fragStatus = Status::failed;
			print(_id, compStatus, vertStatus, geomStatus, fragStatus, linkStatus, std::string(errorLog.begin(), errorLog.end()));
			return false;
		} else fragStatus = Status::success;
	}

	//Link
	program = glCreateProgram();
	if (_compute != nullptr) glAttachShader(program, compute);
	if (_vertex != nullptr) glAttachShader(program, vertex);
	if (_geom != nullptr) glAttachShader(program, geom);
	if (_frag != nullptr) glAttachShader(program, frag);

	glLinkProgram(program);

	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE) {
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
		std::vector<GLchar> errorLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &errorLog[0]);
		if (compute != -1)glDeleteShader(compute);
		if (vertex != -1)glDeleteShader(vertex);
		if (geom != -1)glDeleteShader(geom);
		if (frag != -1)glDeleteShader(frag);
		if (program != -1) glDeleteProgram(program);
		linkStatus = Status::failed;
		print(_id, compStatus, vertStatus, geomStatus, fragStatus, linkStatus, std::string(errorLog.begin(), errorLog.end()));
		return false;
	} else linkStatus = Status::success;

	if (_compute != nullptr)glDetachShader(program, compute);
	if (_vertex != nullptr)glDetachShader(program, vertex);
	if (_geom != nullptr)glDetachShader(program, geom);
	if (_frag != nullptr)glDetachShader(program, frag);

	print(_id, compStatus, vertStatus, geomStatus, fragStatus, linkStatus, "");
	unbind();
	return true;
}

GLuint ShaderProgram::getHandle() {
	return program;
}

bool ShaderProgram::loadFromMemory(const std::string& _id, const std::string& _compute, const std::string& _vertex, const std::string& _geom, const std::string& _frag) {
	return compile(_id, _compute.empty() ? nullptr : _compute.c_str(), _vertex.empty() ? nullptr : _vertex.c_str(), _geom.empty() ? nullptr : _geom.c_str(), _frag.empty() ? nullptr : _frag.c_str());
}

void ShaderProgram::load() {

	bool cExists = true;
	bool vExists = true;
	bool gExists = true;
	bool fExists = true;

	std::ifstream comp(id + ".comp");
	cExists = comp.good();

	std::ifstream vert(id + ".vert");
	vExists = vert.good();

	std::ifstream geom(id + ".geom");
	gExists = geom.good();

	std::ifstream frag(id + ".frag");
	fExists = frag.good();

	data = new std::tuple<std::string, std::string, std::string, std::string>(
		(cExists ? std::string{ std::istreambuf_iterator<char>(comp), std::istreambuf_iterator<char>() } : ""),
		(vExists ? std::string{ std::istreambuf_iterator<char>(vert), std::istreambuf_iterator<char>() } : ""),
		(gExists ? std::string{ std::istreambuf_iterator<char>(geom), std::istreambuf_iterator<char>() } : ""),
		(fExists ? std::string{ std::istreambuf_iterator<char>(frag), std::istreambuf_iterator<char>() } : ""));

	comp.close();
	vert.close();
	geom.close();
	frag.close();
}

bool ShaderProgram::glLoad(void *) {
	std::tuple<std::string, std::string, std::string, std::string>* tuple = reinterpret_cast<std::tuple<std::string, std::string, std::string, std::string>*>(data);

	std::string comp = std::get<0>(*tuple);
	std::string vert = std::get<1>(*tuple);
	std::string geom = std::get<2>(*tuple);
	std::string frag = std::get<3>(*tuple);

	loadFromMemory(id, comp, vert, geom, frag);

	delete tuple;
	isLoaded = true;
	return true;
}

bool ShaderProgram::glUnload(void*) {
	//TODO
	return true;
}

ShaderProgram::ShaderProgram(std::string _id) : Ressource(_id, Type::shader) {}

void ShaderProgram::bind() {
	glUseProgram(getHandle());
	GLError("ShaderProgram::bind");
}

void ShaderProgram::unbind() {
	glUseProgram(0);
	GLError("ShaderProgram::unbind");
}

ShaderProgram* ShaderProgram::get(std::string _id) {
	return M_Asset->get<ShaderProgram*>(_id);
}

ShadowMap::ShadowMap(std::string _id, Framebuffer* _fb, std::string _colorId, std::string _depthId) :
	Ressource(_id, Type::shadowMap), fb(_fb), colorId(_colorId), depthId(_depthId) {}

bool ShadowMap::glUnload(void*) {
	delete fb;
	return true;
}

Texture2D* ShadowMap::getTex() {
	return fb->getTex(colorId);
}

Texture2D* ShadowMap::getDepth() {
	return fb->getTex(depthId);
}

ShadowMap* ShadowMap::get(std::string _id) {
	return M_Asset->get<ShadowMap*>(_id);
}

void ShadowMap::bind() {
	fb->bind();
}

void ShadowMap::unbind() {
	fb->unbind();
}

Vec2u ShadowMap::getBounds() {
	return fb->bounds;
}

bool Framebuffer::glLoad(void*) {
	for (auto& it : textures)
		if (!it.second->loaded()) return false;

	glGenFramebuffers(1, &handle);
	glBindFramebuffer(GL_FRAMEBUFFER, handle);
	uint k = 0;
	//https://www.khronos.org/opengl/wiki/GLAPI/glFramebufferTexture
	for (auto& it : textures){
		Texture2D* tex = it.second;
		if (k == 0)
			bounds = Vec2u(tex->bounds);
		if (tex->format == GL_DEPTH_COMPONENT || tex->format == GL_DEPTH_STENCIL || tex->format == GL_STENCIL_INDEX)
			glFramebufferTexture2D(GL_FRAMEBUFFER, tex->format, GL_TEXTURE_2D, tex->get(), tex->level);
		else
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + k++, GL_TEXTURE_2D, tex->get(), tex->level);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GLError("ShadowMap::glLoad");
	return true;
}

bool Framebuffer::glUnload(void*) {
	for (auto& it : textures)
		delete it.second;
	glDeleteFramebuffers(1, &handle);
	GLError("Framebuffer::glUnload");
}

Framebuffer::Framebuffer(std::string _id, std::unordered_map<std::string, Texture2D*> _textures) :
	Ressource(_id, Type::framebuffer), textures(_textures) {
}

void Framebuffer::bind() {	
	glBindFramebuffer(GL_FRAMEBUFFER, handle);
	glViewport(0, 0, bounds.x, bounds.y);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GLError("Framebuffer::bind");
}

void Framebuffer::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, M_WIDTH, M_HEIGHT);
	GLError("ShadowMap::unbind");
}

Texture2D* Framebuffer::getTex(std::string _index) {
	return textures[_index];
}

Framebuffer* Framebuffer::get(std::string _id) {
	return M_Asset->get<Framebuffer*>(_id);
}

bool Font::glLoad(void *) {
	std::ifstream ifs(id);
	if (!ifs.good()) throw new std::exception((std::string("can't open file [") + id + std::string("]")).data());
	ifs.seekg(0, std::ios::end);
	dataSize = static_cast<uint>(ifs.tellg());
	ifs.seekg(0, std::ios::beg);
	char* data = new char[dataSize];
	ifs.read(data, dataSize);
	ifs.close();
	font = new sf::Font();
	font->loadFromMemory(data, dataSize);
	isLoaded  = true;
	return true;
}

bool Font::glUnload(void *) {
	delete font;
	return true;
}

Font::Font(std::string _id) : Ressource(_id, Type::font){}

Font* Font::get(std::string _id) {
	return M_Asset->get<Font*>(_id);
}

FlipFlopSSBO::FlipFlopSSBO(std::string _id, bool _autoflip, uint _bufferCount, uint _size, GLbitfield _bufferFlags, GLbitfield _mapFlags) : 
	Ressource(_id, Type::ssbo), size(_size), flags(_mapFlags){
	buffers.resize(_bufferCount);
	pointers.resize(_bufferCount);
	for (uint i = 0; i < _bufferCount; ++i)
		buffers[i] = new SSBO(_id + "_" + std::to_string(i), _size, nullptr, _bufferFlags);
	if (_autoflip)
		M_Main->addJob([&](void*)->bool {
		flip();
		return false;
	}, nullptr);
}

void FlipFlopSSBO::flip() {
	index = (index++) % size;
}

bool FlipFlopSSBO::glLoad(void*) {
	bool finishLoading = true;
	for (uint i = 0; i < buffers.size(); ++i) {
		SSBO* buffer = buffers[i];
		if (!buffer->loaded()) {
			finishLoading = false;
			continue;
		}
		pointers[i] = buffer->map(0, dataSize, flags);
	}
	if(finishLoading) isLoaded = true;
	return finishLoading;
}

bool FlipFlopSSBO::glUnload(void*) {
	for (auto b : buffers) {
		b->unmap(); //TODO is that needed??
		delete b;
	}
	return true;
}

void FlipFlopSSBO::bind(uint _binding) {
	bindAs(GL_SHADER_STORAGE_BUFFER, _binding);
}

void FlipFlopSSBO::bindAs(uint _target, uint _binding) {	
	buffers[index]->bindAs(_target, _binding);
}

void FlipFlopSSBO::unbind() {
	buffers[index]->unbind();
}

HeightMap::HeightMap(std::string _id, uint _width, uint _height) : Ressource(_id, Type::heightmap),
	width(_width), height(_height){
	dataSize = _width * _height * sizeof(ushort);	
}

void HeightMap::load() {
	buffer = new FlipFlopSSBO(id + "_buffer", false, 2, dataSize, 
		GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT,
		GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
}

bool HeightMap::glLoad(void*) {
	if (!buffer->loaded()) return false;
	M_Main->addJob([&](void*)->bool {
		ushort* data = buffer->getPtr<ushort*>();
		for (auto it = changeList.begin(), bit = changeList.before_begin(); it != changeList.end(); ++it, ++bit) {
			auto el = *it;
			data[std::get<1>(*el)] = std::get<2>(*el);
			if (std::get<0>(*el)) {
				changeList.erase_after(bit);
				delete el;
			} else std::get<0>(*el) = true;
		}
		buffer->flip();
		return false;
	}, nullptr);
	return true;
}

bool HeightMap::glUnload(void*) {
	delete buffer;
	return true;
}

void HeightMap::bind(uint _binding) {
	buffer->bind(_binding);
}

void HeightMap::unbind() {
	buffer->unbind();
}

void HeightMap::changeEntry(uint _posX, uint _posY, ushort _val) {
	std::tuple<bool, uint, ushort>* tuple = new std::tuple<bool, uint, ushort>(false, _posY * width + _posX, _val);
	changeList.push_front(tuple);
}
