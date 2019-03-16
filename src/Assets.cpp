
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

bool Heerbann::AssetManager::exists(std::string _id) {
	std::lock_guard<std::mutex> guard(assetLock);
	return assets.count(_id) != 0;
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
			sf::Image* tex = new sf::Image();
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
		case Type::atlas:
		{
			TextureAtlasLoader loader;
			next->data = loader(next->id);
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

sf::Sprite* AtlasRegion::createSprite() {
	if (sprite != nullptr) return sprite;
	sprite = new sf::Sprite();
	sprite->setTextureRect(sf::IntRect(sf::Vector2i(x, y), sf::Vector2i(width, height)));
	return sprite;
}

sf::Vector2f AtlasRegion::getU() {
	return sf::Vector2f(1.f / (float)tex->getSize().x * (float) x, 1.f / (float)tex->getSize().x * (float)(x + width));
}

sf::Vector2f AtlasRegion::getV() {
	return sf::Vector2f(1.f / (float)tex->getSize().y * (float)y, 1.f / (float)tex->getSize().y * (float)(y + height));
}

AtlasRegion* TextureAtlas::operator[](std::string _id) {
	if (regions.count(_id) == 0) std::exception((std::string("region does not exist [") + _id + std::string("]")).c_str());
	return regions[_id];
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
				sf::Image* img = new sf::Image();
				img->loadFromFile(_id.substr(0, _id.find_last_of("/") + 1) + line);
				atlas->img.emplace_back(img);
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
				cR->texIndex = imgNr;
				atlas->regions[line] = cR;
				continue;
			}
			break;
			case 2: 
				++regionNr;
				continue;
			case 3: //xy
			{
				++regionNr;
				int pos1 = line.find_first_of(":") + 2;
				int pos2 = line.find_first_of(",");
				int pos3 = line.length() - (pos2 + 1);

				auto x = line.substr(pos1, pos2 - pos1);
				auto y = line.substr(pos2 + 2, pos3 + 1);

				cR->x = std::stoi(x);
				cR->y = std::stoi(y);
			}
			break;
			case 4: //size
			{
				++regionNr;
				int pos1 = line.find_first_of(":") + 2;
				int pos2 = line.find_first_of(",");
				int pos3 = line.length() - (pos2 + 1);

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

	file.close();
	return atlas;
}