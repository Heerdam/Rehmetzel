
#include "MainStruct.hpp"

#include "InputMultiplexer.hpp"
#include "World.hpp"
#include "Assets.hpp"
#include "CameraUtils.hpp"
#include "UI.hpp"

using namespace Heerbann;

MainStruct::MainStruct() {
	inputListener = new InputMultiplexer();
	world = new World();
	assets = new AssetManager();
	stage = new UI::Stage();
};