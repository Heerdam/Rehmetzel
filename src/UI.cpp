
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

UI::StaticLabel::StaticLabel(std::string _id, std::wstring _text, float _size = 500.f, Text::Align _align = Text::Align::right) {
	block = Main::getFontCache()->addStatic(_id, _text, _size, _align);
}

bool UI::StaticLabel::isLoaded() {
	return block == nullptr || block->isLoaded;
}

void UI::StaticLabel::setPosition(sf::Vector2f _pos) {
	if (!isLoaded()) return;
	block->widgetPos = position = _pos;
}

void UI::StaticLabel::draw(SpriteBatch* _batch) {
	if (!isLoaded()) return;
	_batch->draw(block);
}
