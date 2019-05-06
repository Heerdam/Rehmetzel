
#include <fstream>

#include "Assets.hpp"
#include "Level.h"
#include "TextUtil.hpp"
#include "Utils.hpp"
#include "TimeLog.hpp"
#include "Gdx.hpp"

using namespace Heerbann;

LoadItem* AssetManager::popLoad() {
	std::lock_guard<std::mutex> guard(loadQueueLock);
	if (continuousLoadQueue.empty()) return nullptr;
	LoadItem* item = continuousLoadQueue.front();
	continuousLoadQueue.pop();
	return item;
}

LoadItem* AssetManager::popUnload() {
	std::lock_guard<std::mutex> guard(unloadQueueLock);
	if (continuousUnloadQueue.empty()) return nullptr;
	LoadItem* item = continuousUnloadQueue.front();
	continuousUnloadQueue.pop();
	return item;
}

Level* AssetManager::popLevelLoad() {
	std::lock_guard<std::mutex> guard(loadQueueLock);
	if (continuousLevelLoadQueue.empty()) return nullptr;
	Level* item = continuousLevelLoadQueue.front();
	continuousLevelLoadQueue.pop();
	return item;
}

Level* AssetManager::popLevelUnload() {
	std::lock_guard<std::mutex> guard(unloadQueueLock);
	if (continuousLevelUnloadQueue.empty()) return nullptr;
	Level* item = continuousLevelUnloadQueue.front();
	continuousLevelUnloadQueue.pop();
	return item;
}

bool AssetManager::isContinuousLoadQueueEmpty() {
	std::lock_guard<std::mutex> guard(loadQueueLock);
	bool empty = continuousLoadQueue.empty();
	return empty;
}

bool AssetManager::iscontinuousUnloadQueueEmpty() {
	std::lock_guard<std::mutex> guard(unloadQueueLock);
	bool empty = continuousUnloadQueue.empty();
	return empty;
}

bool AssetManager::iscontinuousLevelLoadQueueEmpty() {
	std::lock_guard<std::mutex> guard(loadLevelQueueLock);
	bool empty = continuousLevelLoadQueue.empty();
	return empty;
}

bool AssetManager::iscontinuousLevelUnloadQueueEmpty() {
	std::lock_guard<std::mutex> guard(unloadLevelQueueLock);
	bool empty = continuousLevelUnloadQueue.empty();
	return empty;
}

void AssetManager::queueLoad(LoadItem* _item) {
	std::lock_guard<std::mutex> guard(loadQueueLock);
	_item->isLocked = true;
	continuousLoadQueue.emplace(_item);
}

void AssetManager::queueUnLoad(LoadItem* _item) {
	std::lock_guard<std::mutex> guard(unloadQueueLock);
	_item->isLocked = true;
	continuousUnloadQueue.emplace(_item);
}

void AssetManager::queueLoad(Level* _level) {
	std::lock_guard<std::mutex> guard(loadLevelQueueLock);
	_level->isLocked = true;
	continuousLevelLoadQueue.emplace(_level);
}

void AssetManager::queueUnLoad(Level* _level) {
	std::lock_guard<std::mutex> guard(unloadLevelQueueLock);
	_level->isLocked = true;
	continuousLevelUnloadQueue.emplace(_level);
}

void AssetManager::addAsset(std::string _id, Type _type) {
	addAsset(new LoadItem(_id, _type));	
}

void AssetManager::addAsset(LoadItem* _item) {
	std::lock_guard<std::mutex> guard(assetLock);
	if (assets.count(_item->id) != 0) throw new std::exception(std::string("Asset already exists [").append(_item->id).append("]").c_str());
	assets[_item->id] = _item;
}

AssetManager::AssetManager() {}

AssetManager::~AssetManager() {}

LoadItem* AssetManager::getAsset(std::string _id) {
	std::lock_guard<std::mutex> guard(assetLock);
	if (assets.count(_id) == 0) throw new std::exception(std::string("Asset doesnt exists [").append(_id).append("]").c_str());
	LoadItem* item = assets[_id];
	return item;
}

bool AssetManager::exists(std::string _id) {
	std::lock_guard<std::mutex> guard(assetLock);
	return assets.count(_id) != 0;
}

Level* AssetManager::getLevel(std::string _id) {
	std::lock_guard<std::mutex> guard(levelLock);
	if (levels.count(_id) == 0) throw new std::exception(std::string("Level doesnt exists [").append(_id).append("]").c_str());
	Level* level = levels[_id];
	return level;
}

Level* AssetManager::getLoadedLevel(std::string _id) {
	std::lock_guard<std::mutex> guard(levelLock);
	if (levels.count(_id) == 0) return nullptr;
	auto level = levels[_id];
	return level->isLoaded ? level : nullptr;
}

void AssetManager::load(std::string _id) {
	if (isLoading() && state == discrete) throw new std::exception("cant add to loading queue while loading and in discrete mode");
	LoadItem* item = getAsset(_id);
	if (item == nullptr || item->isLoaded) throw new std::exception(std::string("Asset does not exist or is already loaded [").append(_id).append("]").c_str());
	if(state == continuous) queueLoad(item);
	else discreteLoadQueue.emplace(item);
}

void AssetManager::unload(std::string _id) {
	if (isLoading() && state == discrete) throw new std::exception("cant add to unloading queue while loading and in discrete mode");
	LoadItem* item = (*this)[_id];
	if (item == nullptr) throw new std::exception(std::string("Asset does not exist [").append(_id).append("]").c_str());
	if (state == continuous) queueUnLoad(item);
	else discreteUnloadQueue.emplace(item);
}

void Heerbann::AssetManager::addLevel(std::string _id, Level* _level) {
	std::lock_guard<std::mutex> guard(levelLock);
	if (levels.count(_id) != 0) throw new std::exception(std::string("Level already exists [").append(_id).append("]").c_str());
	levels[_id] = _level;
}

void Heerbann::AssetManager::loadLevel(std::string _id) {
	if (isLoading() && state == discrete) throw new std::exception("cant add to loading queue while loading and in discrete mode");
	Level* item = getLevel(_id);
	if (item == nullptr || item->isLoaded) throw new std::exception(std::string("Level does not exist or is already loaded [").append(_id).append("]").c_str());
	if (state == continuous) queueLoad(item);
	else discreteLevelLoadQueue.emplace(item);
}

void Heerbann::AssetManager::unloadLevel(std::string _id) {
	if (isLoading() && state == discrete) throw new std::exception("cant add to unloading queue while loading and in discrete mode");
	Level* item = getLevel(_id);
	if (item == nullptr || !item->isLoaded) throw new std::exception(std::string("Level does not exist or is already unloaded [").append(_id).append("]").c_str());
	if (state == continuous) queueUnLoad(item);
	else discreteLevelUnloadQueue.emplace(item);
}

void Heerbann::AssetManager::startLoading() {
	if (state == continuous) throw new std::exception("state needs to be discrete!");
	if (locked) throw new std::exception("already loading!");
	locked = true;
	loadingThread = new std::thread(&AssetManager::asyncDiscreteLoad, this);	
}

bool Heerbann::AssetManager::isLoading() {
	return locked;
}

void Heerbann::AssetManager::asyncContinuousLoad() {
	std::unique_lock<std::mutex> guard(cvLock);
	/**
	while (state == continuous) {
		cv.wait(guard, [&]()->bool {
			return !isContinuousLoadQueueEmpty() ||
				!iscontinuousUnloadQueueEmpty() ||
				!iscontinuousLevelLoadQueueEmpty() ||
				!iscontinuousLevelUnloadQueueEmpty();
		});
		
			while (!isContinuousLoadQueueEmpty()) {
				LoadItem* next = popLoad();
				switch (next->type) {
				case Type::texture:
				{
					sf::Texture* tex = new sf::Texture();
					tex->loadFromFile(next->id);
					next->data = tex;
					next->isLoaded = true;
					next->isLocked = false;
				}
				break;
				case Type::font:
				{
					sf::Font* font = new sf::Font();
					font->loadFromFile(next->id);
					next->data = font;
					next->isLoaded = true;
					next->isLocked = false;
				}
				break;				
				}
			}

		while (!iscontinuousUnloadQueueEmpty()) {
			LoadItem* next = popUnload();
			delete next->data;
			next->isLoaded = false;
			next->isLocked = false;
		}

		while (!iscontinuousLevelLoadQueueEmpty()) {
			Level* level = popLevelLoad();
			levelLoader(level);
			level->isLoaded = true;
			level->isLocked = false;
		}

		while (!iscontinuousLevelUnloadQueueEmpty()) {
			Level* level = popLevelUnload();
			levelUnloader(level);
			level->isLoaded = false;
			level->isLocked = false;
		}

	}
	guard.unlock();
	*/
}

void AssetManager::levelLoader(Level* _level) {
	_level->load();
}

void AssetManager::levelUnloader(Level * _level) {
	_level->unload();
}

void AssetManager::asyncDiscreteLoad() {
	progress = 0;
	float inc = 1.f / (float)(discreteLoadQueue.size() + discreteUnloadQueue.size());

	struct WorkOrder {
		//texture_png
		std::function<void(char*, int, LoadItem*, std::atomic<bool>&)> pngLoader;
		char* data;
		int size;
		LoadItem* item;
		//atlas
		std::function<void(char*, int, LoadItem*, int, std::atomic<bool>&)> atlasLoader;
		int index;
	};

	std::atomic<bool> workThreadFinished = false;
	std::queue<WorkOrder> orders;
	std::mutex queueLock;

	std::thread scheduler([&]()->void {
		using namespace std::chrono_literals;

		const int tc = 10;

		std::unique_lock<std::mutex> lock(queueLock);
		lock.unlock();

		std::thread* threads[tc] = { 0 };
		std::atomic<bool> status[tc];

		for (int i = 0; i < tc; ++i)
			status[i] = true;

		while (!workThreadFinished || !orders.empty()) {

			while (orders.empty() && !workThreadFinished) {
				std::this_thread::sleep_for(0.5ms);
			}
						
			if (orders.empty() && workThreadFinished) {
				break;
			}

			lock.lock();
			//std::cout << orders.size() << std::endl;
			WorkOrder order = orders.front();
			orders.pop();			
			lock.unlock();
		
			int index = 0;
			while (true) {
				bool ready = false;
				for (index = 0; index < tc; ++index)
					if (ready = status[index])
						break;
				if (ready) break;
				std::this_thread::sleep_for(0.5ms);
			}

			//for (int i = 0; i < tc; ++i)
				//std::cout << status[i] << " ";
			//std::cout << std::endl << std::endl;

			if (threads[index] != nullptr && threads[index]->joinable()) {
				threads[index]->join();
				delete threads[index];
			}
				
			status[index] = false;

			if (order.pngLoader != nullptr) {			
				threads[index] = new std::thread(order.pngLoader, order.data, order.size, order.item, std::ref(status[index]));
			} else if (order.atlasLoader != nullptr) {
				threads[index] = new std::thread(order.atlasLoader, order.data, order.size, order.item, order.index, std::ref(status[index]));
			}
		}

		for (int i = 0; i < tc; ++i) {
			if (threads[i] != nullptr) {
				if (threads[i]->joinable())
					threads[i]->join();
				delete threads[i];
			}
				
		}

	});

	//thread to load files into memory and dispatch work threads
	while (!discreteLoadQueue.empty()) {
		LoadItem* next = discreteLoadQueue.front();
		discreteLoadQueue.pop();
		std::unique_lock<std::mutex> lock(queueLock);
		lock.unlock();
		switch (next->type) {
			case Type::texture_png:
			{
				std::ifstream ifs(next->id, std::ios::binary);
				if (!ifs.good()) std::exception((std::string("can't open file [") + next->id + std::string("]")).data());			

				//length
				ifs.seekg(0, std::ios::end);
				int length = (int)ifs.tellg();
				ifs.seekg(0, std::ios::beg);

				char* data = new char[length];

				//auto start = std::chrono::system_clock::now();
				ifs.read(data, length);
				auto end = std::chrono::system_clock::now();
				//std::chrono::duration<double> elapsed_seconds = end - start;
				//std::cout << elapsed_seconds.count() << std::endl;

				ifs.close();

				WorkOrder order;
				order.data = data;
				order.item = next;
				order.size = length;
				order.pngLoader = [](char* _data, int _size, LoadItem* _item, std::atomic<bool>& _ready)->void {				
					sf::Image* tex = new sf::Image();
					tex->loadFromMemory(_data, _size);
					_item->data = tex;
									
					M_Main->addJob([](void* _entry)->void {						
						LoadItem* item = (LoadItem*)_entry;
						std::cout << "Loading: [png] " << item->id << std::endl;
						
						sf::Image* im = reinterpret_cast<sf::Image*>(item->data);
						sf::Texture* tex = new sf::Texture();
						tex->loadFromImage(*im);
						delete im;
						item->data = tex;
						item->isLoaded = true;
						item->isLocked = false;
					}, _item);
					_ready = true;
				};
				
				lock.lock();
				orders.emplace(order);
				lock.unlock();
			}
			break;
			case Type::texture_dds:
			{
				//TODO
			}
			break;
			case Type::shader:
			{
				
				bool cExists = true;
				bool vExists = true;
				bool gExists = true;
				bool fExists = true;

				std::ifstream comp(next->id + ".comp");
				cExists = comp.good();

				std::ifstream vert(next->id + ".vert");
				vExists = vert.good();

				std::ifstream geom(next->id + ".geom");
				gExists = geom.good();

				std::ifstream frag(next->id + ".frag");
				fExists = frag.good();
				
				std::tuple<std::string, std::string, std::string, std::string, LoadItem*>* tuple = new std::tuple<std::string, std::string, std::string, std::string, LoadItem*>(
					(cExists ? std::string{ std::istreambuf_iterator<char>(comp), std::istreambuf_iterator<char>() } : ""),
					(vExists ? std::string{ std::istreambuf_iterator<char>(vert), std::istreambuf_iterator<char>() } : ""),
					(gExists ? std::string{ std::istreambuf_iterator<char>(geom), std::istreambuf_iterator<char>() } : ""),
					(fExists ? std::string{ std::istreambuf_iterator<char>(frag), std::istreambuf_iterator<char>() } : ""),
					next
					);

				comp.close();
				vert.close();
				geom.close();
				frag.close();

				M_Main->addJob([](void* _entry)->void {
					std::tuple<std::string, std::string, std::string, std::string, LoadItem*>* tuple = reinterpret_cast<std::tuple<std::string, std::string, std::string, std::string, LoadItem*>*>(_entry);

					std::string comp = std::get<0>(*tuple);
					std::string vert = std::get<1>(*tuple);
					std::string geom = std::get<2>(*tuple);
					std::string frag = std::get<3>(*tuple);
					LoadItem* item = std::get<4>(*tuple);
					
					ShaderProgram* shader = new ShaderProgram();
					shader->loadFromMemory(item->id, comp, vert, geom, frag);

					delete tuple;
					item->data = shader;
					item->isLoaded = true;
					item->isLocked = false;
				}, tuple);
			}
			break;
			case Type::font:
			{
				std::ifstream ifs(next->id);
				if (!ifs.good()) throw new std::exception((std::string("can't open file [") + next->id + std::string("]")).data());

				//length
				ifs.seekg(0, std::ios::end);
				int length = (int)ifs.tellg();
				ifs.seekg(0, std::ios::beg);

				char* data = new char[length];
				next->data = data;

				ifs.read(data, length);
				ifs.close();

				std::tuple<LoadItem*, int>* tuple = new std::tuple<LoadItem*, int>(next, length);

				M_Main->addJob([](void* _entry)->void {
					//std::cout << "started finishing font" << std::endl;
					std::tuple<LoadItem*, int>* tuple = (std::tuple<LoadItem*, int>*)_entry;
					sf::Font* font = new sf::Font();
					LoadItem* item = std::get<0>(*tuple);
					font->loadFromMemory(item->data, std::get<1>(*tuple));
					delete tuple;
					item->data = font;
					item->isLoaded = true;
					item->isLocked = false;
					//std::cout << "ended finishing font" << std::endl;
				}, tuple);
			}
			break;
			case Type::atlas:
			{
				TextureAtlasLoader loader;
				TextureAtlas* atlas = loader(next->id);
				next->data = atlas;

				for (unsigned int i = 0; i < atlas->files.size(); ++i) {

					std::ifstream ifs(atlas->files[i], std::ios::binary);
					if (!ifs.good()) throw new std::exception((std::string("can't open file [") + atlas->files[i] + std::string("]")).data());

					//length
					ifs.seekg(0, std::ios::end);
					int length = (int)ifs.tellg();
					ifs.seekg(0, std::ios::beg);

					char* data = new char[length];

					ifs.read(data, length);
					ifs.close();

					WorkOrder order;
					order.data = data;
					order.item = next;
					order.size = length;
					order.index = (int)i;
					order.atlasLoader = [](char* _data, int _size, LoadItem* _item, int _index, std::atomic<bool>& _ready)->void {
						//std::cout << "started loading atlas [" << _index << "]" << std::endl;
						TextureAtlas* atlas = (TextureAtlas*)_item->data;
						atlas->img[_index] = new sf::Image();
						atlas->img[_index]->loadFromMemory(_data, _size);

						std::tuple<LoadItem*, int>* tuple = new std::tuple<LoadItem*, int>(_item, _index);

						M_Main->addJob([](void* _entry)->void {
							//std::cout << "started finishing atlas" << std::endl;
							std::tuple<LoadItem*, int>* tuple = (std::tuple<LoadItem*, int>*)_entry;
							int index = std::get<1>(*tuple);
							LoadItem* item = std::get<0>(*tuple);
							TextureAtlas* atlas = (TextureAtlas*)item->data;

							atlas->tex[index] = new sf::Texture();
							atlas->tex[index]->loadFromImage(*atlas->img[index]);
							delete atlas->img[index];
							atlas->img[index] = nullptr;

							delete tuple;
							//std::cout << "ended finishing atlas" << std::endl;
							for (unsigned int i = 0; i < atlas->tex.size(); ++i) {
								if (atlas->tex[i] == nullptr) return;
							}

							item->isLoaded = true;
							item->isLocked = false;

						}, tuple);
						//std::cout << "ended loading atlas [" << _index << "]" << std::endl;
						_ready = true;
					};
					
					lock.lock();
					orders.emplace(order);
					lock.unlock();
				}
			}
			break;
			case model:
			{		

				Assimp::Importer importer;
				const aiScene *scene = importer.ReadFile(next->id, aiProcess_ValidateDataStructure | aiProcess_FixInfacingNormals |
					aiProcess_FlipUVs | aiProcess_FlipWindingOrder);

				unsigned int severity = 0;
				severity |= Assimp::Logger::VERBOSE;
				// Detach debug messages from you self defined stream
				Assimp::DefaultLogger::get()->attachStream(new myStream(), severity);
			
				//vec3 pos
				//vec3 normals
				//vev2 uv
				//float index
				std::vector<float> vertexBuffer;
				std::vector<unsigned int> indexBuffer;
				 //offset, size

				Model* modelOut = new Model();
				next->data = modelOut;

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

					mat->Get(AI_MATKEY_OPACITY, material.OPACITY);

					mat->Get(AI_MATKEY_SHININESS, material.SHININESS);

					mat->Get(AI_MATKEY_SHININESS_STRENGTH, material.SHININESS_STRENGTH);

					materialList[i] = material;
				}

				//Vertex & Index data
				int meshIndexOffset = 0;
				int meshVertexOffset = 0;
				//meshes

				modelOut->meshList.resize(scene->mNumMeshes);

				for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
					
					aiMesh* mesh = scene->mMeshes[i];
					Mesh* meshOut = new Mesh();
					modelOut->meshList[i] = meshOut;
					modelOut->meshMap[std::string(mesh->mName.C_Str())] = meshOut;

					meshOut->vertexCount = mesh->mNumVertices;
					meshOut->vertexOffset = static_cast<uint>(vertexBuffer.size());

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
						//index
						vertexBuffer.emplace_back(static_cast<float>(mesh->mMaterialIndex));

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



					/*
					weights.resize(indexBuffer.size());
					boneCache.resize(mesh->mNumBones);

					boneCount += mesh->mNumBones;

					//weights
					for (unsigned int k = 0; k < mesh->mNumBones; ++k) {
						auto bone = mesh->mBones[k];
						boneCache[k] = std::string(bone->mName.C_Str());
						boneIndices[std::string(bone->mName.C_Str())] = k;
						for (unsigned int j = 0; j < bone->mNumWeights; ++j) {
							unsigned int index = static_cast<unsigned int>(bone->mWeights[j].mVertexId);
							if (weights[index] == nullptr)
								weights[index] = new std::vector<std::tuple<unsigned int, float>*>();
							weights[index]->emplace_back(new std::tuple<unsigned int, float>(index, bone->mWeights[j].mWeight));
						}	

					}
					*/

				}




				//numIndex
				//offset_to_weights[index]
				//offset_to_animation[animation_index]
				//offset_to_bones[animation_index]

				//weights [index]: {bone_index, weight}
				//animation [animation_index][bone_index][time]: {mat4}

				//int boneCount = 0;
				//std::vector<aiString> boneCache;
				//std::unordered_map<std::string, unsigned int> boneIndices;

				//weights [index]: {bone_index, weight}
				//std::vector<std::vector<std::tuple<unsigned int, float>*>*> weights;



















				modelOut->vertexBufferCacheSize = static_cast<uint>(vertexBuffer.size());
				modelOut->vertexBufferCache = new float[vertexBuffer.size()];
				std::memcpy(modelOut->vertexBufferCache, vertexBuffer.data(), vertexBuffer.size() * sizeof(float));

				modelOut->indexBufferCacheSize = static_cast<uint>(indexBuffer.size());
				modelOut->indexBufferCache = new unsigned int[indexBuffer.size()];
				std::memcpy(modelOut->indexBufferCache, indexBuffer.data(), indexBuffer.size() * sizeof(unsigned int));

				modelOut->materialCacheSize = static_cast<uint>(materialList.size());
				modelOut->materialCache = new char[sizeof(Material) * materialList.size()];
				std::memcpy(modelOut->materialCache, indexBuffer.data(), materialList.size() * sizeof(Material));

				M_Main->addJob([](void* _entry)->void {
					LoadItem* item = reinterpret_cast<LoadItem*>(_entry);
					Model* model = reinterpret_cast<Model*>(item->data);
					LOG("Loading: [Model] " + item->id);
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

					//material
					glBindBuffer(GL_SHADER_STORAGE_BUFFER, model->matBuffer);
					glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Material) * model->materialCacheSize, model->materialCache, GL_STATIC_DRAW);
					glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, model->matBuffer);

					glEnableVertexAttribArray(0);
					glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);

					glEnableVertexAttribArray(1);
					glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));

					glEnableVertexAttribArray(2);
					glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));

					glEnableVertexAttribArray(3);
					glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(8 * sizeof(float)));
				
					glBindVertexArray(0);

					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
					glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

					if (model->vertexBufferCache != nullptr) delete model->vertexBufferCache;
					if (model->indexBufferCache != nullptr) delete model->indexBufferCache;
					if (model->animationCache != nullptr) delete model->animationCache;
					if (model->materialCache != nullptr) delete model->materialCache;

					item->isLoaded = true;
					item->isLocked = false;

					App::Gdx::printOpenGlErrors(item->id);

				}, next);
			
				/*
				//animation [animation_index][bone_index]: {time, mat4}
				std::vector<std::vector<std::vector<std::tuple<double, Matrix4>*>*>*> animation;
				std::unordered_map<std::string, aiAnimation*> animations;

				//animations
				for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
					aiAnimation* an = scene->mAnimations[i];
					animations[std::string(an->mName.C_Str())] = an;
	
					for (unsigned int k = 0; k < an->mNumChannels; ++k) {
						auto* channel = an->mChannels[k];

						unsigned int i1 = 0;
						unsigned int i2 = 0;
						unsigned int i3 = 0;

						for (double j = 0; j < an->mDuration; ++an) {
							bool changed = false;

							if (i1 + 1 < channel->mNumPositionKeys) {
								if (channel->mPositionKeys[i1 + 1].mTime == j) {
									++i1;
									changed = true;
								}
							}

							if (i1 + 1 < channel->mNumRotationKeys) {
								if (channel->mRotationKeys[i2 + 1].mTime == j) {
									++i2;
									changed = true;
								}
							}

							if (i3 + 1 < channel->mNumScalingKeys) {
								if (channel->mScalingKeys[i3 + 1].mTime == j) {
									++i3;
									changed = true;
								}
							}
							
							if (!changed) continue;

							Matrix4 mat(channel->mPositionKeys[i1].mValue, channel->mRotationKeys[i2].mValue, channel->mScalingKeys[i3].mValue);

							if (animation[i] == nullptr) {
								animation[i] = new std::vector<std::vector<std::tuple<double, Matrix4>*>*>();
								animation[i]->resize(boneCount);
							}
							
							auto an1 = *animation[i];

							unsigned int boneIndex = boneIndices[std::string(channel->mNodeName.C_Str())];

							if (an1[boneIndex] == nullptr) {
								an1[boneIndex] = new std::vector<std::tuple<double, Matrix4>*>();
							}

							an1[boneIndex]->emplace_back(new std::tuple<double, Matrix4>(j, mat));
						}
											
					}
				}*/
			
			}
			break;
		}
		progress = 1.0;
	}

	while (!discreteUnloadQueue.empty()) {
		LoadItem* next = discreteUnloadQueue.front();
		discreteUnloadQueue.pop();
		delete next->data;
		next->isLoaded = false;
		next->isLocked = false;
		progress = std::clamp(inc + progress, 0.f, 1.f);
	}

	while (!discreteLevelLoadQueue.empty()) {
		Level* level = discreteLevelLoadQueue.front();
		discreteLevelLoadQueue.pop();
		levelLoader(level);
		level->isLoaded = true;
		level->isLocked = false;
	}

	while (!discreteLevelUnloadQueue.empty()) {
		Level* level = discreteLevelUnloadQueue.front();
		discreteLevelUnloadQueue.pop();
		levelUnloader(level);
		level->isLoaded = false;
		level->isLocked = false;
	}

	workThreadFinished = true;
	if (scheduler.joinable()) {
		scheduler.join();
	}
	progress = 1;
	locked = false;
}

void AssetManager::finish() {
	if (state == continuous) throw new std::exception("state needs to be discrete!");
	if(locked) throw new std::exception("already loading!");
	locked = true;
	loadingThread = new std::thread(&AssetManager::asyncDiscreteLoad, this);
	if (loadingThread->joinable())
		loadingThread->join();
	M_Main->update();
	progress = 1;
}

void Heerbann::AssetManager::toggleState() {
	/*
	if (state == discrete) {
		state = continuous;
		if (loadingThread->joinable())
			loadingThread->join();
		loadingThread = std::thread(&AssetManager::asyncContinuousLoad, this);
	} else {
		state = discrete;
		if (loadingThread.joinable())
			loadingThread.join();
	}
	*/
}

sf::Sprite* AtlasRegion::createSprite() {
	if (sprite != nullptr) return sprite;
	sprite = new sf::Sprite();
	sprite->setTextureRect(sf::IntRect(sf::Vector2i(x, y), sf::Vector2i(width, height)));
	return sprite;
}

Vec2 AtlasRegion::getU() {
	return Vec2(1.f / (float)parent->tex[texIndex]->getSize().x * (float) x, 1.f / (float)parent->tex[texIndex]->getSize().x * (float)(x + width));
}

Vec2 AtlasRegion::getV() {
	return Vec2(1.f / (float)parent->tex[texIndex]->getSize().y * (float)y, 1.f / (float)parent->tex[texIndex]->getSize().y * (float)(y + height));
}

AtlasRegion* TextureAtlas::operator[](std::string _id) {
	if (regions.count(_id) == 0) throw new std::exception((std::string("region does not exist [") + _id + std::string("]")).c_str());
	return regions[_id];
}

AtlasRegion * Heerbann::TextureAtlas::operator[](int _index) {
	return regionList[_index];
}

TextureAtlas* TextureAtlasLoader::operator()(std::string _id) {
	TextureAtlas* atlas = new TextureAtlas();
	std::ifstream file(_id + std::string(".atlas"));

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
				atlas->files.emplace_back(_id.substr(0, _id.find_last_of("/") + 1) + line);
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
				cR->parent = atlas;
				cR->texIndex = imgNr;
				atlas->regions[line] = cR;
				atlas->regionList.emplace_back(cR);
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

	atlas->tex.resize(atlas->files.size());
	atlas->img.resize(atlas->files.size());
	file.close();
	return atlas;
}