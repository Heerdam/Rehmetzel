
#include <fstream>

#include "Assets.hpp"
#include "Level.h"
#include "TextUtil.hpp"
#include "Utils.hpp"

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
	std::lock_guard<std::mutex> guard(assetLock);
	if (assets.count(_item->id) != 0) std::exception(std::string("Asset already exists [").append(_item->id).append("]").c_str());	
	assets[_item->id] = _item;
}

Text::StaticTextBlock* AssetManager::loadStaticText(std::string _id, std::wstring _text, float _width, Text::Align _align) {
	std::lock_guard<std::mutex> guard(assetLock);
	if (assets.count(_id) != 0) std::exception(std::string("Asset already exists [").append(_id).append("]").c_str());
	auto item = new LoadItem(_id, static_text);
	assets[_id] = item;
	auto text = Main::getFontCache()->createStatic(_id);
	text->block->setText(_text);
	text->block->setWidth(_width);
	text->block->setAlign(_align);
	item->data = text;
	if (item == nullptr || item->isLoaded) std::exception(std::string("Asset does not exist or is already loaded [").append(_id).append("]").c_str());
	//if (state == continuous) queueLoad(item);
	else discreteLoadQueue.emplace(item);
	return text;
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

void Heerbann::AssetManager::levelLoader(Level* _level) {
	_level->load(this);
}

void Heerbann::AssetManager::levelUnloader(Level * _level) {
	_level->unload(this);
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
					//std::cout << "started loading png " << _item->id << std::endl;
					sf::Image* tex = new sf::Image();
					tex->loadFromMemory(_data, _size);
					_item->data = tex;
					
					
					Main::addJob([](void* _entry)->void {
						//std::cout << "started finishing png" << std::endl;
						LoadItem* item = (LoadItem*)_entry;
						sf::Image* im = (sf::Image*)item->data;
						sf::Texture* tex = new sf::Texture();
						tex->loadFromImage(*im);
						delete im;
						item->data = tex;
						item->isLoaded = true;
						item->isLocked = false;
						//std::cout << "ended finishing png" << std::endl;
					}, _item);
					//std::cout << "ended loading png " << _item->id << std::endl;
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
				bool gExists = true;

				std::ifstream vert(next->id + ".vert");
				if (!vert.good()) std::exception((std::string("can't open file [") + next->id + std::string(".vert]")).data());

				std::ifstream geom(next->id + ".geom");
				if (!geom.good()) {
					gExists = false;
					
				}

				std::ifstream frag(next->id + ".frag");
				if (!frag.good()) std::exception((std::string("can't open file [") + next->id + std::string(".frag]")).data());

				
				std::tuple<std::string, std::string, std::string, LoadItem*>* tuple = new std::tuple<std::string, std::string, std::string, LoadItem*>(
					std::string{ std::istreambuf_iterator<char>(vert), std::istreambuf_iterator<char>() },
					(gExists ? std::string{ std::istreambuf_iterator<char>(geom), std::istreambuf_iterator<char>() } : ""),
					std::string{ std::istreambuf_iterator<char>(frag), std::istreambuf_iterator<char>() },
					next
					);
				/*
				std::tuple<std::string, std::string, std::string, LoadItem*>* tuple = new std::tuple<std::string, std::string, std::string, LoadItem*>(
					next->id + std::string(".vert"),
					gExists ? next->id + std::string(".geom") : std::string(""),
					next->id + std::string(".frag"),
					next
					);
					*/
				vert.close();
				geom.close();
				frag.close();

				Main::addJob([](void* _entry)->void {
					std::tuple<std::string, std::string, std::string, LoadItem*>* tuple = (std::tuple<std::string, std::string, std::string, LoadItem*>*)_entry;

					std::string vert = std::get<0>(*tuple);
					std::string geom = std::get<1>(*tuple);
					std::string frag = std::get<2>(*tuple);
					LoadItem* item = std::get<3>(*tuple);
					
					auto asset = Main::getAssetManager();
					sf::Shader* shader = new sf::Shader();
					if (!geom.empty())
						shader->loadFromMemory(vert, geom, frag);
					else 
						shader->loadFromMemory(vert, frag);

					//GLenum err;
					//while ((err = glGetError()) != GL_NO_ERROR)
						//std::cout << err << std::endl;
					//GLint linked;
					//glGetProgramiv(shader->getNativeHandle(), GL_LINK_STATUS, &linked);
					//std::cout << linked << std::endl;
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
				if (!ifs.good()) std::exception((std::string("can't open file [") + next->id + std::string(".vert]")).data());

				//length
				ifs.seekg(0, std::ios::end);
				int length = (int)ifs.tellg();
				ifs.seekg(0, std::ios::beg);

				char* data = new char[length];
				next->data = data;

				ifs.read(data, length);
				ifs.close();

				std::tuple<LoadItem*, int>* tuple = new std::tuple<LoadItem*, int>(next, length);

				Main::addJob([](void* _entry)->void {
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
					if (!ifs.good()) std::exception((std::string("can't open file [") + atlas->files[i] + std::string("]")).data());

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

						Main::addJob([](void* _entry)->void {
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
			case static_text:
			{
				Text::StaticTextBlock* block = reinterpret_cast<Text::StaticTextBlock*>(next->data);
				block->block->layoutAndRebuild(TYP_FONT_STATIC);
				next->data = block;
				Main::addJob([](void* _entry)->void {
					LoadItem* item = reinterpret_cast<LoadItem*>(_entry);
					Main::getFontCache()->sendToGPU(reinterpret_cast<Text::StaticTextBlock*>(item->data));
					item->isLoaded = true;
					item->isLocked = false;
				}, next);
				
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
	if (state == continuous) std::exception("state needs to be discrete!");
	if(locked) std::exception("already loading!");
	locked = true;
	loadingThread = new std::thread(&AssetManager::asyncDiscreteLoad, this);
	if (loadingThread->joinable())
		loadingThread->join();
	Main::get()->update();
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

sf::Vector2f AtlasRegion::getU() {
	return sf::Vector2f(1.f / (float)parent->tex[texIndex]->getSize().x * (float) x, 1.f / (float)parent->tex[texIndex]->getSize().x * (float)(x + width));
}

sf::Vector2f AtlasRegion::getV() {
	return sf::Vector2f(1.f / (float)parent->tex[texIndex]->getSize().y * (float)y, 1.f / (float)parent->tex[texIndex]->getSize().y * (float)(y + height));
}

AtlasRegion* TextureAtlas::operator[](std::string _id) {
	if (regions.count(_id) == 0) std::exception((std::string("region does not exist [") + _id + std::string("]")).c_str());
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

	atlas->tex.resize(atlas->files.size());
	atlas->img.resize(atlas->files.size());
	file.close();
	return atlas;
}