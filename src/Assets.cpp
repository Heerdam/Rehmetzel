
#include <fstream>

#include "Assets.hpp"
#include "Level.h"

using namespace Heerbann;

AssetManager::LoadItem* AssetManager::popLoad() {
	std::unique_lock<std::mutex> guard(loadQueueLock);
	guard.lock();
	if (continuousLoadQueue.empty()) {
		guard.unlock();
		return nullptr;
	}
	LoadItem* item = continuousLoadQueue.front();
	continuousLoadQueue.pop();
	guard.unlock();
	return item;
}

AssetManager::LoadItem* AssetManager::popUnload() {
	std::unique_lock<std::mutex> guard(unloadQueueLock);
	guard.lock();
	if (continuousUnloadQueue.empty()) {
		guard.unlock();
		return nullptr;
	}
	LoadItem* item = continuousUnloadQueue.front();
	continuousUnloadQueue.pop();
	guard.unlock();
	return item;
}

Level * Heerbann::AssetManager::popLevelLoad() {
	std::unique_lock<std::mutex> guard(loadQueueLock);
	guard.lock();
	if (continuousLevelLoadQueue.empty()) {
		guard.unlock();
		return nullptr;
	}
	Level* item = continuousLevelLoadQueue.front();
	continuousLevelLoadQueue.pop();
	guard.unlock();
	return item;
}

Level * Heerbann::AssetManager::popLevelUnload() {
	std::unique_lock<std::mutex> guard(unloadQueueLock);
	guard.lock();
	if (continuousLevelUnloadQueue.empty()) {
		guard.unlock();
		return nullptr;
	}
	Level* item = continuousLevelUnloadQueue.front();
	continuousLevelUnloadQueue.pop();
	guard.unlock();
	return item;
}

bool Heerbann::AssetManager::isContinuousLoadQueueEmpty() {
	std::unique_lock<std::mutex> guard(loadQueueLock);
	guard.lock();
	bool empty = continuousLoadQueue.empty();
	guard.unlock();
	return empty;
}

bool Heerbann::AssetManager::iscontinuousUnloadQueueEmpty() {
	std::unique_lock<std::mutex> guard(unloadQueueLock);
	guard.lock();
	bool empty = continuousUnloadQueue.empty();
	guard.unlock();
	return empty;
}

bool Heerbann::AssetManager::iscontinuousLevelLoadQueueEmpty() {
	std::unique_lock<std::mutex> guard(loadLevelQueueLock);
	guard.lock();
	bool empty = continuousLevelLoadQueue.empty();
	guard.unlock();
	return empty;
}

bool Heerbann::AssetManager::iscontinuousLevelUnloadQueueEmpty() {
	std::unique_lock<std::mutex> guard(unloadLevelQueueLock);
	guard.lock();
	bool empty = continuousLevelUnloadQueue.empty();
	guard.unlock();
	return empty;
}

void AssetManager::queueLoad(LoadItem* _item) {
	std::unique_lock<std::mutex> guard(loadQueueLock);
	guard.lock();
	_item->isLocked = true;
	continuousLoadQueue.emplace(_item);
	guard.unlock();
}

void AssetManager::queueUnLoad(LoadItem* _item) {
	std::unique_lock<std::mutex> guard(unloadQueueLock);
	guard.lock();
	_item->isLocked = true;
	continuousUnloadQueue.emplace(_item);
	guard.unlock();
}

void Heerbann::AssetManager::queueLoad(Level* _level) {
	std::unique_lock<std::mutex> guard(loadLevelQueueLock);
	guard.lock();
	_level->isLocked = true;
	continuousLevelLoadQueue.emplace(_level);
	guard.unlock();
}

void Heerbann::AssetManager::queueUnLoad(Level* _level) {
	std::unique_lock<std::mutex> guard(unloadLevelQueueLock);
	guard.lock();
	_level->isLocked = true;
	continuousLevelUnloadQueue.emplace(_level);
	guard.unlock();
}

void AssetManager::addAsset(std::string _id, Type _type) {
	std::unique_lock<std::mutex> guard(assetLock);
	guard.lock();	
	if(assets.count(_id) != 0) std::exception(std::string("Asset already exists [").append(_id).append("]").c_str());
	assets[_id] = new LoadItem(_id, _type);	
	guard.unlock();
}

AssetManager::LoadItem* AssetManager::getAsset(std::string _id) {
	std::unique_lock<std::mutex> guard(assetLock);
	guard.lock();
	if (assets.count(_id) == 0) std::exception(std::string("Asset doesnt exists [").append(_id).append("]").c_str());
	LoadItem* item = assets[_id];
	guard.unlock();
	return item;
}

Level * Heerbann::AssetManager::getLevel(std::string _id) {
	std::unique_lock<std::mutex> guard(levelLock);
	guard.lock();
	if (levels.count(_id) == 0) std::exception(std::string("Level doesnt exists [").append(_id).append("]").c_str());
	Level* level = levels[_id];
	guard.unlock();
	return level;
}

Level * Heerbann::AssetManager::getLoadedLevel(std::string _id) {
	std::unique_lock<std::mutex> guard(levelLock);
	guard.lock();
	if (levels.count(_id) == 0) return nullptr;
	auto level = levels[_id];
	guard.unlock();
	return level->isLoaded ? level : nullptr;
}

void AssetManager::load(std::string _id) {
	if (isLoading() && state == discrete) std::exception("cant add to loading queue while loading and in discrete mode");
	LoadItem* item = (*this)[_id];
	if (item == nullptr) std::exception(std::string("Asset does not exist [").append(_id).append("]").c_str());
	queueLoad(item);
}

void AssetManager::unload(std::string _id) {
	if (isLoading() && state == discrete) std::exception("cant add to unloading queue while loading and in discrete mode");
	LoadItem* item = (*this)[_id];
	if (item == nullptr) std::exception(std::string("Asset does not exist [").append(_id).append("]").c_str());
	queueUnLoad(item);
}

void Heerbann::AssetManager::addLevel(std::string _id, Level* _level) {
	std::unique_lock<std::mutex> guard(levelLock);
	guard.lock();
	if (levels.count(_id) != 0) std::exception(std::string("Level already exists [").append(_id).append("]").c_str());
	levels[_id] = _level;
	guard.unlock();
}

void Heerbann::AssetManager::loadLevel(std::string _id) {
	if (isLoading() && state == discrete) std::exception("cant add to loading queue while loading and in discrete mode");
	Level* item = getLevel(_id);
	if (item == nullptr || item->isLoaded) std::exception(std::string("Level does not exist or is already loaded [").append(_id).append("]").c_str());
	queueLoad(item);
}

void Heerbann::AssetManager::unloadLevel(std::string _id) {
	if (isLoading() && state == discrete) std::exception("cant add to unloading queue while loading and in discrete mode");
	Level* item = getLevel(_id);
	if (item == nullptr || !item->isLoaded) std::exception(std::string("Asset does not exist or is already unloaded [").append(_id).append("]").c_str());
	queueUnLoad(item);
}

void Heerbann::AssetManager::startLoading() {
	if (state == continuous) std::exception("state needs to be discrete!");
	if (locked) std::exception("already loading!");
	locked = true;
	loadingThread = std::thread(&AssetManager::asyncDiscreteLoad, this);	
}

bool Heerbann::AssetManager::isLoading() {
	return locked;
}

void Heerbann::AssetManager::asyncContinuousLoad() {
	std::unique_lock<std::mutex> guard(cvLock);

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
				case Type::shader:
				{
					std::ifstream vert(next->id + ".vert");
					std::ifstream frag(next->id + ".frag");
					std::ifstream geom(next->id + ".geom");
					bool v = vert.good();
					bool f = frag.good();
					bool g = geom.good();
					vert.close();
					frag.close();
					geom.close();
					sf::Shader* shader = new sf::Shader();
					if (v) shader->loadFromFile(next->id + ".vert", sf::Shader::Type::Vertex);
					if (f) shader->loadFromFile(next->id + ".frag", sf::Shader::Type::Fragment);
					if (g) shader->loadFromFile(next->id + ".geom", sf::Shader::Type::Geometry);
					next->data = shader;
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
}

void Heerbann::AssetManager::levelLoader(Level * _level) {
	//TODO
}

void Heerbann::AssetManager::levelUnloader(Level * _level) {
	//TODO
}

void AssetManager::asyncDiscreteLoad() {
	progress = 0;
	float inc = 1.f / (float)(discreteLoadQueue.size() + discreteUnloadQueue.size());

	while (!discreteLoadQueue.empty()) {
		LoadItem* next = discreteLoadQueue.front();
		discreteLoadQueue.pop();
		switch (next->type) {
		case Type::texture:
		{
			sf::Texture* tex = new sf::Texture();
			tex->loadFromFile(next->id);
			next->data = tex;
			next->isLoaded = true;
			next->isLocked = false;
			progress = std::clamp(inc + progress, 0.f, 1.f);
		}
		break;
		case Type::font:
		{
			sf::Font* font = new sf::Font();
			font->loadFromFile(next->id);
			next->data = font;
			next->isLoaded = true;
			next->isLocked = false;
			progress = std::clamp(inc + progress, 0.f, 1.f);
		}
		break;
		case Type::shader:
		{
			std::ifstream vert(next->id + ".vert");
			std::ifstream frag(next->id + ".frag");
			std::ifstream geom(next->id + ".geom");
			bool v = vert.good();
			bool f = frag.good();
			bool g = geom.good();
			vert.close();
			frag.close();
			geom.close();
			sf::Shader* shader = new sf::Shader();
			if(v) shader->loadFromFile(next->id + ".vert", sf::Shader::Type::Vertex);
			if(f) shader->loadFromFile(next->id + ".frag", sf::Shader::Type::Fragment);
			if(g) shader->loadFromFile(next->id + ".geom", sf::Shader::Type::Geometry);
			next->data = shader;
			next->isLoaded = true;
			next->isLocked = false;
			progress = std::clamp(inc + progress, 0.f, 1.f);
		}
		break;
		}
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
	progress = 1;
	locked = false;
}

void AssetManager::finish() {
	if (state == continuous) std::exception("state needs to be discrete!");
	if(locked) std::exception("already loading!");
	locked = true;
	loadingThread = std::thread(&AssetManager::asyncDiscreteLoad, this);
	loadingThread.join();
	progress = 1;
}

void Heerbann::AssetManager::changeState(State _state) {
	if(isLoading()) std::exception("state cant be changed while loading!");
}

