#pragma once

#include<vector>
#include<unordered_map>
#include<queue>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "MainStruct.hpp"

namespace Heerbann {

	using namespace Heerbann;

	class AssetManager;
	namespace Text {
		struct StaticTextBlock;
		class FontCache;
		enum Align : int;
	}

	struct Level;

	enum Type {
		texture_png, texture_dds, font, level, atlas, shader, static_text
	};

	enum State {
		continuous, discrete
	};

	struct LoadItem {
		const long long uniqueId = Main::getId();
		std::atomic<bool> isLocked = false;

		bool isLoaded = false;
		Type type;
		std::string id;
		void* data;
		LoadItem(std::string _id, Type _type) : id(_id), type(_type) {};
	};

	class AssetManager {

		friend Text::FontCache;

	private:
		std::unordered_map<std::string, LoadItem*> assets;
		std::unordered_map<std::string, Level*> levels;

		State state = State::discrete;
		std::atomic<bool> locked = false;

		std::thread* loadingThread;

		//discrete loading
		std::queue<LoadItem*> discreteLoadQueue;
		std::queue<LoadItem*> discreteUnloadQueue;
		std::queue<Level*> discreteLevelLoadQueue;
		std::queue<Level*> discreteLevelUnloadQueue;
		std::atomic<float> progress;


		//continous loading
		std::queue<LoadItem*> continuousLoadQueue;
		std::queue<LoadItem*> continuousUnloadQueue;
		std::queue<Level*> continuousLevelLoadQueue;
		std::queue<Level*> continuousLevelUnloadQueue;

		std::mutex loadQueueLock;
		std::mutex unloadQueueLock;
		std::mutex loadLevelQueueLock;
		std::mutex unloadLevelQueueLock;

		std::mutex assetLock;
		std::mutex levelLock;

		std::mutex cvLock;
		std::condition_variable cv;
		
		//thread safe functions for continous loading
		LoadItem* popLoad();
		LoadItem* popUnload();
		Level* popLevelLoad();
		Level* popLevelUnload();

		bool isContinuousLoadQueueEmpty();
		bool iscontinuousUnloadQueueEmpty();
		bool iscontinuousLevelLoadQueueEmpty();
		bool iscontinuousLevelUnloadQueueEmpty();

		void queueLoad(LoadItem*);
		void queueUnLoad(LoadItem*);
		void queueLoad(Level*);
		void queueUnLoad(Level*);

		//loading functions
		void asyncDiscreteLoad();
		void asyncContinuousLoad();

		void levelLoader(Level* _level);
		void levelUnloader(Level* _level);

		//creates a static text entry in the fontcache (thread safe)
		Text::StaticTextBlock* loadStaticText(std::string, std::wstring, float, Text::Align);
		
	public:
		//thread safe method to get Asset
		LoadItem* operator[](std::string _id) {
			std::lock_guard<std::mutex> guard(assetLock);
			if (assets.count(_id) == 0) return nullptr;
			auto asset = assets[_id];
			return asset->isLoaded ? asset : nullptr;
		};

		//get asset if exists (thread safe)
		LoadItem* getAsset(std::string);

		//check if asset exists (thread safe)
		bool exists(std::string);

		//get a loaded level (thread safe)
		Level* getLoadedLevel(std::string);

		//get a level if exists (thread safe)
		Level* getLevel(std::string);

		//adds an asset to the manager (thread safe)
		void addAsset(std::string, Type);

		//adds an asset to the manager (thread safe)
		void addAsset(LoadItem*);

		//enqueues a new asset to load (discrete, continuous) (thread safe)
		void load(std::string);

		//enqueues a new asset to unload (discrete, continuous) (thread safe)
		void unload(std::string);

		//adds a level to the manager (thread safe)
		void addLevel(std::string, Level*);

		//adds a level to the loading queue (discrete, continuous) (thread safe)
		void loadLevel(std::string);

		//adds a level to the unloading queue (discrete, continuous) (thread safe)
		void unloadLevel(std::string);

		//begins loading (discrete) (thread safe)
		void startLoading();

		//is currently loading (discrete, continuous) (thread safe)
		bool isLoading();

		//blocks threat until loading is finished (discrete loading) (thread safe)
		void finish();

		//changes the state of the loader (thread safe)
		void toggleState();

		inline State getState() {
			return state;
		};

	};

	struct TextureAtlas;

	struct AtlasRegion {
		TextureAtlas* parent;
		sf::Sprite* sprite;
		int texIndex = -1;
		int x, y, width, height;

		sf::Sprite* createSprite();
		sf::Vector2f getU();
		sf::Vector2f getV();
	};

	struct TextureAtlas {
		std::vector<sf::Texture*> tex;
		std::vector<sf::Image*> img;
		std::vector<std::string> files;
		std::unordered_map<std::string, AtlasRegion*> regions;
		std::vector<AtlasRegion*> regionList;
		AtlasRegion* operator[](std::string);
		AtlasRegion* operator[](int);
	};

	class TextureAtlasLoader {
	public:
		TextureAtlas* operator()(std::string);
	};

}
