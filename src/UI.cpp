
#include "UI.hpp"

#include "Utils.hpp"
#include "InputMultiplexer.hpp"
#include "CameraUtils.hpp"
#include "TextUtil.hpp"

using namespace Heerbann;
using namespace UI;


void UI::Stage::act() {
	for (auto c : children)
		c->act();
}

void UI::Stage::draw(SpriteBatch* _batch) {
	for (auto c : children)
		c->draw(_batch);
}

const Vec2 & UI::Actor::getPosition() {
	return position;
}
