
#include <fstream>

#include "Assets.hpp"
#include "Level.h"

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

Level * Heerbann::AssetManager::popLevelLoad() {
	std::lock_guard<std::mutex> guard(loadQueueLock);
	if (continuousLevelLoadQueue.empty()) return nullptr;
	Level* item = continuousLevelLoadQueue.front();
	continuousLevelLoadQueue.pop();
	return item;
}

Level * Heerbann::AssetManager::popLevelUnload() {
	std::lock_guard<std::mutex> guard(unloadQueueLock);
	if (continuousLevelUnloadQueue.empty()) return nullptr;
	Level* item = continuousLevelUnloadQueue.front();
	continuousLevelUnloadQueue.pop();
	return item;
}

bool Heerbann::AssetManager::isContinuousLoadQueueEmpty() {
	std::lock_guard<std::mutex> guard(loadQueueLock);
	bool empty = continuousLoadQueue.empty();
	return empty;
}

bool Heerbann::AssetManager::iscontinuousUnloadQueueEmpty() {
	std::lock_guard<std::mutex> guard(unloadQueueLock);
	bool empty = continuousUnloadQueue.empty();
	return empty;
}

bool Heerbann::AssetManager::iscontinuousLevelLoadQueueEmpty() {
	std::lock_guard<std::mutex> guard(loadLevelQueueLock);
	bool empty = continuousLevelLoadQueue.empty();
	return empty;
}

bool Heerbann::AssetManager::iscontinuousLevelUnloadQueueEmpty() {
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

void Heerbann::AssetManager::queueLoad(Level* _level) {
	std::lock_guard<std::mutex> guard(loadLevelQueueLock);
	_level->isLocked = true;
	continuousLevelLoadQueue.emplace(_level);
}

void Heerbann::AssetManager::queueUnLoad(Level* _level) {
	std::lock_guard<std::mutex> guard(unloadLevelQueueLock);
	_level->isLocked = true;
	continuousLevelUnloadQueue.emplace(_level);
}

void AssetManager::addAsset(std::string _id, Type _type) {
	addAsset(new LoadItem(_id, _type));	
}

void Heerbann::AssetManager::addAsset(LoadItem* _item) {
	if (assets.count(_item->id) != 0) std::exception(std::string("Asset already exists [").append(_item->id).append("]").c_str());
	std::lock_guard<std::mutex> guard(assetLock);
	assets[_item->id] = _item;
}

LoadItem* AssetManager::getAsset(std::string _id) {
	std::lock_guard<std::mutex> guard(assetLock);
	if (assets.count(_id) == 0) std::exception(std::string("Asset doesnt exists [").append(_id).append("]").c_str());
	LoadItem* item = assets[_id];
	return item;
}

Level * Heerbann::AssetManager::getLevel(std::string _id) {
	std::lock_guard<std::mutex> guard(levelLock);
	if (levels.count(_id) == 0) std::exception(std::string("Level doesnt exists [").append(_id).append("]").c_str());
	Level* level = levels[_id];
	return level;
}

Level * Heerbann::AssetManager::getLoadedLevel(std::string _id) {
	std::lock_guard<std::mutex> guard(levelLock);
	if (levels.count(_id) == 0) return nullptr;
	auto level = levels[_id];
	return level->isLoaded ? level : nullptr;
}

void AssetManager::load(std::string _id) {
	if (isLoading() && state == discrete) std::exception("cant add to loading queue while loading and in discrete mode");
	LoadItem* item = getAsset(_id);
	if (item == nullptr || item->isLoaded) std::exception(std::string("Asset does not exist or is already loaded [").append(_id).append("]").c_str());
	if(state == continuous) queueLoad(item);
	else discreteLoadQueue.emplace(item);
}

void AssetManager::unload(std::string _id) {
	if (isLoading() && state == discrete) std::exception("cant add to unloading queue while loading and in discrete mode");
	LoadItem* item = (*this)[_id];
	if (item == nullptr) std::exception(std::string("Asset does not exist [").append(_id).append("]").c_str());
	if (state == continuous) queueUnLoad(item);
	else discreteUnloadQueue.emplace(item);
}

void Heerbann::AssetManager::addLevel(std::string _id, Level* _level) {
	std::lock_guard<std::mutex> guard(levelLock);
	if (levels.count(_id) != 0) std::exception(std::string("Level already exists [").append(_id).append("]").c_str());
	levels[_id] = _level;
}

void Heerbann::AssetManager::loadLevel(std::string _id) {
	if (isLoading() && state == discrete) std::exception("cant add to loading queue while loading and in discrete mode");
	Level* item = getLevel(_id);
	if (item == nullptr || item->isLoaded) std::exception(std::string("Level does not exist or is already loaded [").append(_id).append("]").c_str());
	if (state == continuous) queueLoad(item);
	else discreteLevelLoadQueue.emplace(item);
}

void Heerbann::AssetManager::unloadLevel(std::string _id) {
	if (isLoading() && state == discrete) std::exception("cant add to unloading queue while loading and in discrete mode");
	Level* item = getLevel(_id);
	if (item == nullptr || !item->isLoaded) std::exception(std::string("Level does not exist or is already unloaded [").append(_id).append("]").c_str());
	if (state == continuous) queueUnLoad(item);
	else discreteLevelUnloadQueue.emplace(item);
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

void Heerbann::AssetManager::levelLoader(Level* _level) {
	_level->load(this);
}

void Heerbann::AssetManager::levelUnloader(Level * _level) {
	_level->unload(this);
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

			if (v && f && g) {
				if (!shader->loadFromFile(next->id + ".vert", next->id + ".geom", next->id + ".frag"))
					std::exception("vertex/ geom/ fragment failed");
			} else if (v && f) {
				if (!shader->loadFromFile(next->id + ".vert", next->id + ".frag"))
					std::exception("vertex/ fragment failed");
			} else std::exception("cant load shader");

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
	if (loadingThread.joinable())
		loadingThread.join();
	progress = 1;
}

void Heerbann::AssetManager::toggleState() {
	if (state == discrete) {
		state = continuous;
		if (loadingThread.joinable())
			loadingThread.join();
		loadingThread = std::thread(&AssetManager::asyncContinuousLoad, this);
	} else {
		state = discrete;
		if (loadingThread.joinable())
			loadingThread.join();
	}
}

