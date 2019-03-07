
#include "Assets.hpp"

using namespace Heerbann;

void AssetManager::load(std::string _id, Type _type) {
	if (isLoading) std::exception("Assetmanager is currently loading. Nothing can be added");
	LoadItem* item = (*this)[_id];
	if (item != nullptr) std::exception(std::string("Asset already exists [").append(_id).append("]").c_str());
	loadQueue.emplace(new LoadItem(_id, _type));
}

void AssetManager::unload(std::string _id) {
	if (isLoading) std::exception("Assetmanager is currently loading. Nothing can be removed");
	LoadItem* item = (*this)[_id];
	if (item == nullptr) std::exception(std::string("Asset does not exist [").append(_id).append("]").c_str());
	unloadQueue.emplace(item);
}

void AssetManager::asyncLoad() {
	float inc = 1.f / (float)(loadQueue.size() + unloadQueue.size());
	while (!loadQueue.empty()) {
		LoadItem* next = loadQueue.front();
		loadQueue.pop();
		switch (next->type) {
		case Type::texture:
		{
			sf::Texture* tex = new sf::Texture();
			tex->loadFromFile(next->id);
			next->data = tex;
			assets[next->id] = next;
			progress = std::clamp(inc + progress, 0.f, 1.f);
		}
		break;
		}
	}
	while (!unloadQueue.empty()) {
		LoadItem* next = unloadQueue.front();
		loadQueue.pop();
		assets.erase(next->id);
		delete next->data;
		delete next;
		progress = std::clamp(inc + progress, 0.f, 1.f);
	}
}

float AssetManager::update() {
	if (!isLoading) {
		isLoading = true;
		t1 = std::thread(&AssetManager::asyncLoad, this);
		progress = 0;
	}
	return progress;
}

void AssetManager::finish() {
	if (!isLoading)
		t1 = std::thread(&AssetManager::asyncLoad, this);
	t1.join();
	isLoading = false;
	progress = 1;
};
