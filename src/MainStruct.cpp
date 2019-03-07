
#include "MainStruct.hpp"

#include "InputMultiplexer.hpp"
#include "World.hpp"
#include "Assets.hpp"
#include "CameraUtils.hpp"

using namespace Heerbann;

MainStruct::MainStruct() {
	inputListener = new InputMultiplexer();
	world = new World();
	assets = new AssetManager();
};