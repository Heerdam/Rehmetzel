
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

void Texture2D::glLoad(void*) {
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
}

void Texture2D::glUnload(void*) {
	glDeleteTextures(1, &handle);
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

void Array2DTexture::glLoad(void*) {
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
}

void Array2DTexture::glUnload(void*) {
	glDeleteTextures(1, &handle);
	GLError("Array2DTexture::glUnload");
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

void TextureAtlas::glLoad(void*) {
	//TODO
}

void TextureAtlas::glUnload(void*) {
	//TODO
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

	ModelData* modelOut = new ModelData();
	data = modelOut;

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

	modelOut->meshList.resize(scene->mNumMeshes);

	modelOut->boneCache.resize(scene->mNumMeshes);

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {

		aiMesh* mesh = scene->mMeshes[i];
		Mesh* meshOut = new Mesh();
		modelOut->meshList[i] = meshOut;
		modelOut->meshMap[std::string(mesh->mName.C_Str())] = meshOut;

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

		modelOut->boneCache[i].resize(mesh->mNumBones);
		for (uint k = 0; k < mesh->mNumBones; ++k) {
			auto bone = mesh->mBones[i];
			Bone* out = new Bone();
			modelOut->boneCache[i].emplace_back(out);
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

		modelOut->nodeMap[out->id] = out;
		modelOut->nodeCache.emplace_back(out);
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

	modelOut->vertexBufferCacheSize = static_cast<uint>(vertexBuffer.size());
	modelOut->vertexBufferCache = new float[vertexBuffer.size()];
	std::memcpy(modelOut->vertexBufferCache, vertexBuffer.data(), vertexBuffer.size() * sizeof(float));

	modelOut->indexBufferCacheSize = static_cast<uint>(indexBuffer.size());
	modelOut->indexBufferCache = new unsigned int[indexBuffer.size()];
	std::memcpy(modelOut->indexBufferCache, indexBuffer.data(), indexBuffer.size() * sizeof(unsigned int));

	modelOut->materialCacheSize = static_cast<uint>(materialList.size());
	modelOut->materialCache = new char[sizeof(Material) * materialList.size()];
	std::memcpy(modelOut->materialCache, materialList.data(), materialList.size() * sizeof(Material));

}

void Model::glLoad(void*) {

	ModelData* model = reinterpret_cast<ModelData*>(data);
	LOG("Loading: [Model] " + id);
	GLuint vbo;
	GLuint index;

	//create buffer
	glGenVertexArrays(1, &model->vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &index);
	glGenBuffers(1, &model->matBuffer);

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

	//material
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, model->matBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Material) * model->materialCacheSize, model->materialCache, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	if (model->vertexBufferCache != nullptr) delete model->vertexBufferCache;
	if (model->indexBufferCache != nullptr) delete model->indexBufferCache;
	if (model->animationCache != nullptr) delete model->animationCache;
	if (model->materialCache != nullptr) delete model->materialCache;

	isLoaded = true;
	GLError("Model::glLoad");
}

void Heerbann::Model::glUnload(void *) {
	//TODO
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

void ShaderProgram::glLoad(void *) {
	std::tuple<std::string, std::string, std::string, std::string>* tuple = reinterpret_cast<std::tuple<std::string, std::string, std::string, std::string>*>(data);

	std::string comp = std::get<0>(*tuple);
	std::string vert = std::get<1>(*tuple);
	std::string geom = std::get<2>(*tuple);
	std::string frag = std::get<3>(*tuple);

	loadFromMemory(id, comp, vert, geom, frag);

	delete tuple;
	isLoaded = true;
}

void ShaderProgram::glUnload(void*) {
	//TODO
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

ShadowMap::ShadowMap(std::string _id, Framebuffer* _fb, std::string _shadowID) :
	Ressource(_id, Type::shadowMap), fb(_fb), shadowId(_shadowID) {}

void ShadowMap::glUnload(void*) {
	delete fb;
}

Texture2D* ShadowMap::getTex() {
	return fb->getTex(shadowId);
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

void Framebuffer::glLoad(void*) {
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
}

void Framebuffer::glUnload(void*) {
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

void Font::glLoad(void *) {
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
}

void Font::glUnload(void *) {
	delete font;
}

Font::Font(std::string _id) : Ressource(_id, Type::font){}

Font* Font::get(std::string _id) {
	return M_Asset->get<Font*>(_id);
}

void TextureDebugRenderer::glLoad(void*) {
	shader = new ShaderProgram("assets/shader/fbo_debug_shader");

	float vertices[] = {
		1.f, 1.f, 0.f,
		1.f, 1.f,

		1.f, -1.f, 0.f,
		1.f, 0.f,

		-1.f, -1.f, 0.f,
		0.f, 0.f,

		-1.f, 1.f, 0.f,
		0.f, 1.f
	};

	unsigned int indices[] = {
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

void TextureDebugRenderer::glUnload(void*) {
	//delete shader;
	glDeleteVertexArrays(1, &vao);
	GLError("TextureDebugRenderer::glUnload");
}

TextureDebugRenderer::TextureDebugRenderer() : Renderer("TextureDebugRenderer") {}

void TextureDebugRenderer::add(Renderable* _renderable) {
	renderables.emplace_back(_renderable);
}

void TextureDebugRenderer::add(std::vector<Renderable*> _renderables) {
	renderables.emplace_back(_renderables.begin(), _renderables.end());
}

void TextureDebugRenderer::draw(View* _view) {
	for (Renderable* r : renderables) {
		TxDbgRenderable* rend = reinterpret_cast<TxDbgRenderable*>(r);
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
	if (!_tex->loaded()) return;

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
	if (!_tex->loaded()) return;

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
	//if (!_region->loaded()) return;
	//TODO
}

void TextureDebugRenderer::draw(TextureAtlas* _atlas, uint _index, float) {
	if (!_atlas->loaded()) return;
	//TODO
}

void TextureDebugRenderer::draw(GLuint _texture, GLuint _type, uint _state, uint _width, uint _height, uint _index, uint _viewportW, uint _viewportH) {
	if (!shader->loaded()) return;
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

void VSMRenderer::drawVSM(std::vector<VSMRenderable*>) {
	//draw geometry (with tex)
	vsmShader->bind();
	for (auto g : geometry) {
		Model* model = std::get<4>(g);

		model->bindTransform(3);
		model->bindinvTransform(4);
		_view->bindCombined(5);
		light->bindLightTransform(6, model->position, 1500.f, 250.f);
		M_Env->bindLights(2);
		_view->bindPosition(7);
		glUniform1ui(8, std::get<3>(g));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, std::get<5>(g));

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, std::get<6>(g));

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, model->matBuffer);

		glBindVertexArray(model->vao);
		glDrawElements(GL_TRIANGLES, std::get<2>(g), GL_UNSIGNED_INT, (void*)(std::get<1>(g) * sizeof(uint)));
		glBindVertexArray(0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	vsmShader->unbind();

	GLError("TestWorldLevel::draw::vsm_tex");
}

void VSMRenderer::drawVSMNoTex(std::vector<VSMRenderable*>) {
	//draw geometry (no tex)
	vsmNoTexShader->bind();
	for (auto g : geometryNoTex) {
		Model* model = std::get<3>(g);

		model->bindTransform(3);
		model->bindinvTransform(4);
		_view->bindCombined(5);
		light->bindLightTransform(6, model->position, 1500.f, 250.f);

		_view->bindPosition(7);

		M_Env->bindLights(2);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, std::get<4>(g));

		glBindVertexArray(model->vao);
		glDrawElements(GL_TRIANGLES, std::get<2>(g), GL_UNSIGNED_INT, (void*)(std::get<1>(g) * sizeof(uint)));
		glBindVertexArray(0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	};
	vsmNoTexShader->unbind();

	GLError("TestWorldLevel::draw::vsm_notex");
}

void VSMRenderer::drawVoxel(std::vector<VSMRenderable*>) {
}

VSMRenderer::VSMRenderer(std::string _id) : Renderer(_id) {
	vsmShader = new ShaderProgram("assets/shader/simple forward/sb_sf_vsm");
	vsmNoTexShader = new ShaderProgram("assets/shader/simple forward/sb_sf_vsm_notex");

	debugR = new TextureDebugRenderer();
	shadowR = new ShadowRenderer("shadowVSMRenderer", ShadowRenderer::VSM);
	//blur = new GaussianBlur(128, 8);
}

void VSMRenderer::add(Renderable* _renderable) {
	VSMRenderable* r = reinterpret_cast<VSMRenderable*>(_renderable);
	if (r->hasTex)
		renderables.emplace_back(r);
	else rendNoTex.emplace_back(r);
}

void VSMRenderer::add(std::vector<Renderable*> _renderables) {
	for (auto r : _renderables)
		add(r);
}

void VSMRenderer::draw(View* _view) {

	



	renderables.clear();
	rendNoTex.clear();
}

Renderer::Renderer(std::string _id) : Ressource(_id, Type::renderer) {}

void ShadowRenderer::glLoad(void*) {
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

void ShadowRenderer::glUnload(void*) {
	delete fb;
}

ShadowRenderer::ShadowRenderer(std::string _id, uint _renderType) : Renderer(_id), renderType(_renderType) {}

void ShadowRenderer::add(Renderable* _renderable) {
	renderables.emplace_back(_renderable);
}

void ShadowRenderer::add(std::vector<Renderable*> _renderables) {
	renderables.emplace_back(_renderables.begin(), _renderables.end());
}

void ShadowRenderer::draw(View* _view) {
	auto lights = M_Env->queryLights(_view);

	shader->bind();
	for (auto l : lights) {
		if (l->shadowMap == nullptr) continue;
		l->shadowMap->bind();
		for (auto t : renderables) {
			t->model->bindTransform(2);
			l->bindLightTransform(1, t->model->position, 1500.f, 250.f);//TODO distance for dir light?
			glBindVertexArray(t->vao);
			glDrawElements(GL_TRIANGLES, t->count, GL_UNSIGNED_INT, (void*)(t->offset * sizeof(uint)));
			glBindVertexArray(0);
		}
		l->shadowMap->unbind();
	}
	shader->unbind();

	GLError("TestWorldLevel::draw::shadowmaps");
}

void VoxelBackGroundRenderer::glLoad(void*) {
	shader = new ShaderProgram("assets/shader/voxel/shader_voxel_builder");
}

void VoxelBackGroundRenderer::glUnload(void*) {
	delete shader;
}

VoxelBackGroundRenderer::VoxelBackGroundRenderer(std::string _id) : Renderer(_id) {}

void VoxelBackGroundRenderer::add(Renderable* _renderable) {
	renderables.emplace_back(reinterpret_cast<VoxelRenderable*>(_renderable));
}

void VoxelBackGroundRenderer::draw(View* _view) {
	BoundingBox* aabb = _view->getCamera()->frustum->toAABB(_view->getCamera());
	Vec3u num_groups(VOXELS - 2, VOXELS - 2, 1);

	bSizeIndex = (bSizeIndex + 1) % 2;
	auto pntr = bSizeBufferPntr[bSizeIndex];
	pntr[0] = 0;

	pntr[1] = VOXELS - 2;
	pntr[2] = VOXELS - 2;

	pntr[3] = 0;
	pntr[4] = 0;

	pntr[5] = 50;
	pntr[6] = 5;

	shader->bind();
	for (auto r : renderables) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_RECTANGLE, r->heightmap);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, r->vao);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bSizeBuffer[bSizeIndex]);

		glDispatchCompute(num_groups.x, num_groups.y, num_groups.z);
		glMemoryBarrier(GL_ALL_BARRIER_BITS | GL_SHADER_STORAGE_BARRIER_BIT | GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
	}
	shader->unbind();
	renderables.clear();
	GLError("VoxelBackGroundRenderer::draw");
}
