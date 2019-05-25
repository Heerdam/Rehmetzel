
#include "TextUtil.hpp"
#include "Utils.hpp"
#include "Assets.hpp"
#include "CameraUtils.hpp"
#include "Math.hpp"

using namespace Heerbann;

std::queue<Text::Letter*> Text::Letter::cache = std::queue<Text::Letter*>();
std::queue<Text::Line*> Text::Line::cache = std::queue<Text::Line*>();
std::queue<Text::TextBlock*> Text::FontCache::blockCache = std::queue<Text::TextBlock*>();

Heerbann::Text::Style::Style() {
	font = DefaultFont;
}

Text::Style::Style(Style& _s) {
	align = _s.align;
	font = _s.font;
	fontColor = _s.fontColor;
	outlineColor = _s.outlineColor;
	fontSize = _s.fontSize;
	outlineThickness = _s.outlineThickness;
	bold = _s.bold;
}

Text::TextBlock* Text::FontCache::create(std::wstring _id) {
	TextBlock* block;
	if (!blockCache.empty()){
		block = blockCache.front();
		blockCache.pop();
	} else {
		block = new TextBlock(M_Font);
	}
	M_Font->dynamicBlocks[_id] = block;
	return block;
}

void Text::FontCache::free(TextBlock* _block) {
	_block->reset();
	blockCache.push(_block);
	M_Font->dynamicBlocks.erase(_block->id);
}

Text::FontCache::FontCache() {}

void Text::FontCache::addFont(std::wstring _id, sf::Font* _font) {
	fonts[_id] = _font;
}

//---------------------- Textblock ----------------------\\

Text::TextBlock::TextBlock(Text::FontCache* _parent) {
	Text::TextBlock(_parent, _parent->defaultStyle);
}

Text::TextBlock::TextBlock(Text::FontCache* _parent, Text::Style _style) {
	parent = _parent;
	defaultStyle = _style;
}

void Text::TextBlock::free() {
	parent->free(this);
}

void Text::TextBlock::reset() {
	if (!jobs.empty()) {
		auto j = jobs.front();
		jobs.pop();
		delete j;
	}
	for (auto l : lines)
		l->free();
	lines.clear();
	id.clear();
	defaultStyle = Style(parent->defaultStyle);
	pos = Vec2(0.f, 0.f);
	maxWidth = 250.f;
	aabb = new BoundingBox();
	spriteCount = 0;
	currentText.clear();
}

void Text::TextBlock::setText(std::wstring _text) {
	currentText = _text;
	jobs.push(new Job(TextJob::t_rebuild));
}

void Text::TextBlock::update(std::wstring _text, int _index) {
	//TODO
}

void Text::TextBlock::setAlign(Text::Align _align) {
	align = _align;
	jobs.push(new Job(TextJob::t_rebuild));
}

void Text::TextBlock::setWidth(float _width) {
	maxWidth = _width;
	jobs.push(new Job(TextJob::t_rebuild));
}

int Text::TextBlock::layoutAndRebuild(float* _data, const Vec4& _scissors) {
	int spriteCount = layout(currentText);
	calculateBounds();
	reBuild(_data, _scissors);
	return spriteCount;
}

 void Text::TextBlock::draw(int& _size, float* _data, const Vec4& _scissors) {

	bool rebuilt = false;
	while (!jobs.empty()) {

		Job* job = jobs.front();
		jobs.pop();

		switch (job->jobType) { 
			case TextJob::t_rebuild:
			{
				if (!rebuilt) {
					rebuilt = true;
					spriteCount = layoutAndRebuild(_data, _scissors);
				}
				delete job;
			}
			break;
			case TextJob::t_insert:
			{
				rebuilt = false;
				//TODO
			}
			break;
			case TextJob::t_update:
			{
				rebuilt = false;
				//TODO
			}
			break;
		}

	}

	_size += spriteCount;
}

 BoundingBox* Text::TextBlock::bounds() {
	return aabb;
}

int Text::TextBlock::layout(std::wstring _text) {

	for (auto l : lines) {
		for (auto g : l->letters)
			g->free();
		l->free();
	}
	lines.clear();

	std::vector<BaseLetter*> letterCache;

	Style style(defaultStyle);

	int size = 0;

	//1. pass: find styles & create letters
	for (unsigned int i = 0; i < _text.length(); ++i) {

		//next letter
		uint letter = _text[i];

		//space
		if (letter == 32) {
			Letter* l = Letter::get();
			l->copyStyle(style);
			l->letter = 32;
			letterCache.emplace_back(l);
			continue;
		}

		//line break
		if (letter == 92) {
			if (i < _text.length() - 1 && _text[i + 1] == 110) {
				letterCache.emplace_back(new LineBreak());
				i++;
				continue;
			}
		}

		//beginning of style found
		if (letter == 123) {

			const static std::wstring fn = M_s2ws("fn");
			const static std::wstring fc = M_s2ws("fc");
			const static std::wstring bc = M_s2ws("bc");
			const static std::wstring bt = M_s2ws("bt");
			const static std::wstring sz = M_s2ws("sz");
			const static std::wstring bl = M_s2ws("bl");
			const static std::wstring tr = M_s2ws("true");
			const static std::wstring fl = M_s2ws("false");
			const static std::wstring end = M_s2ws("end");

			int j = static_cast<int>(_text.substr(i + 1).find(M_s2ws("}"))); //TODO
			std::wstring styleString = _text.substr(i + 1, j);
			i += j + 1;

			if (styleString == end) {
				style = Style(defaultStyle);
				continue;
			}

			std::vector<std::wstring> styles = App::Util::split(styleString, M_s2ws(","));

			for (const auto& s : styles) {

				std::wstring type = s.substr(0, 2);
				std::wstring data = s.substr(3);

				if (type.empty() || data.empty())
					continue;

				if (type == fn) {

					//style.font = fonts[data]; //TODO

				} else if (type == fc) {

					int r = std::stoi(data.substr(0, 3));
					int g = std::stoi(data.substr(3, 3));
					int b = std::stoi(data.substr(6, 3));
					int a = std::stoi(data.substr(9, 3));
					style.fontColor = sf::Color(r, g, b, a);

				} else if (type == bc) {

					int r = std::stoi(data.substr(0, 3));
					int g = std::stoi(data.substr(3, 3));
					int b = std::stoi(data.substr(6, 3));
					int a = std::stoi(data.substr(9, 3));
					style.outlineColor = sf::Color(r, g, b, a);

				} else if (type == bt) {

					style.outlineThickness = std::stof(data);

				} else if (type == sz) {

					style.fontSize = std::stoi(data);

				} else if (type == bl) {

					if (data == tr)
						style.bold = true;
					else if (data == fl)
						style.bold = false;

				} else continue;

			}
			continue;
		}

		Letter* l = Letter::get();
		l->copyStyle(style);
		l->letter = letter;
		letterCache.emplace_back(l);

		if (l->outlineThickness != 0)
			size += 2;
		else
			++size;
	}

	spriteCount = size;

	//2. pass: assemble letters on lines
	Line* currentLine = Line::get();
	std::vector<Line*> lineCache;
	lineCache.emplace_back(currentLine);
	float posX = 0;
	float lineSpacing = 0;
	auto batch = M_Batch;
	unsigned int lastWordIndex = -1;
	for (auto l : letterCache) {
		switch (l->type) {
			case BaseLetter::lineBreak:
			{
				currentLine->spacing = lineSpacing;
				currentLine = Line::get();
				lineCache.emplace_back(currentLine);
				posX = 0;
				lineSpacing = 0;
				lastWordIndex = -1;
				delete l;
			}
			break;
			case BaseLetter::letter:
			{
				Letter* letter = (Letter*)l;				
				auto glyph = letter->font->getGlyph(letter->letter, letter->fontSize, letter->bold, letter->outlineThickness);
				letter->texIndex = batch->getIndex(letter->font->getTexture(letter->fontSize).getNativeHandle());
				letter->localPos.x = posX;		
				letter->localBounds->clr()->ext(glyph.bounds);
				posX += glyph.advance;

				if (letter->letter == 32)
					lastWordIndex = static_cast<int>(currentLine->letters.size());

				if (posX > maxWidth) { //line end reached				

					if (lastWordIndex == -1 || lastWordIndex == 0) { //if word longer than line, split at max length

						//TODO

					}else if (lastWordIndex == currentLine->letters.size()){

						if (letter->letter != 32) {
							Line* nextLine = Line::get();
							posX = 0;
							lineSpacing = 0;
							currentLine = nextLine;
							lineCache.emplace_back(currentLine);
							letter->localPos.x = posX;
							posX += glyph.advance;
						}

					} else {

						Line* nextLine = Line::get();
						posX = 0;
						lineSpacing = 0;

						//move word to next line
						
						for (unsigned int i = lastWordIndex + 1; i < currentLine->letters.size(); ++i) {
							Letter* l = currentLine->letters[i];
							glyph = l->font->getGlyph(l->letter, l->fontSize, l->bold, l->outlineThickness);

							l->localPos.x = posX;
							posX += glyph.advance;

							float lsp = l->font->getLineSpacing(l->fontSize);
							lineSpacing = lsp > lineSpacing ? lsp : lineSpacing;
							nextLine->letters.emplace_back(l);
							nextLine->spacing = lineSpacing;
						}

						currentLine->letters.erase(currentLine->letters.begin() + lastWordIndex + 1, currentLine->letters.end());
						lastWordIndex = -1;

						currentLine = nextLine;
						lineCache.emplace_back(currentLine);						
						letter->localPos.x = posX;

						glyph = letter->font->getGlyph(letter->letter, letter->fontSize, letter->bold, letter->outlineThickness);
						posX += glyph.advance;
						//currentLine->letters.emplace_back(letter);
					}
					
				}

				float lsp = letter->font->getLineSpacing(letter->fontSize);
				lineSpacing = lsp > lineSpacing ? lsp : lineSpacing;
				currentLine->letters.emplace_back(letter);
				currentLine->spacing = lineSpacing;
			}
			break;
		}
	}

	//2.5 pass centre if Align::centre
	if (align == Align::centre) {
		for (auto l : lineCache) {
			//get line width
			float width = maxWidth;
			//get actual width
			auto last = l->letters.back();
			float actualWidth = last->localPos.x;
			//find most left centred point
			float offset = (maxWidth - actualWidth) * 0.5f;
			//move everything			
			for (auto letter : l->letters)
				letter->localPos.x += offset;
		}
	} else if (align == Align::right) {
		for (auto l : lineCache) {
			//get line width
			float width = maxWidth;
			//get actual width
			auto last = l->letters.back();
			float actualWidth = last->localPos.x;
			//find most left centred point
			float offset = maxWidth - actualWidth;
			for (auto letter : l->letters)
				letter->localPos.x += offset;
		}
	}

	//3. pass adjust the line spacing
	lines = lineCache;

	float currentHeight = 0;
	for (unsigned int i = 0; i < lines.size(); ++i) {
		lines[i]->localPos.y = i ? currentHeight : 0.f;
		currentHeight += lines[i]->spacing;
	}

	return size;

}

void Text::TextBlock::addQuad(float* _data, int _index, Text::Letter* _letter, float _outline, sf::Color _color, const Vec4& _scissors) {

	auto position = _letter->globalPos;
	auto glyph = _letter->font->getGlyph(_letter->letter, _letter->fontSize, _letter->bold, _outline);

	float fCol = M_FloatBits(_color);
	float oCol = 0.f;

	float padding = 1.0f;
	float outlineThickness = _outline;
	float italicShear = 0;

	Vec2 min(_letter->globalBounds->min.x - padding - outlineThickness,
		_letter->globalBounds->max.y + padding - outlineThickness);

	Vec2 max(_letter->globalBounds->max.x + outlineThickness,
		_letter->globalBounds->min.y + padding + outlineThickness);

	float u1 = static_cast<float>(glyph.textureRect.left) - padding;
	float v1 = static_cast<float>(glyph.textureRect.top) - padding;
	float u2 = static_cast<float>(glyph.textureRect.left + glyph.textureRect.width) + padding;
	float v2 = static_cast<float>(glyph.textureRect.top + glyph.textureRect.height) + padding;

	auto texSize = _letter->font->getTexture(_letter->fontSize).getSize();
	float fracW = 1.f / texSize.x;
	float fracH = 1.f / texSize.y;

	u1 *= fracW;
	u2 *= fracW;
	v1 *= fracH;
	v2 *= fracH;

	int k = 0;
	//bottom left
	_data[k++] = min.x;
	_data[k++] = min.y;

	_data[k++] = u1;
	_data[k++] = v2;
	_data[k++] = (float)_letter->texIndex;

	_data[k++] = fCol;

	_data[k++] = _scissors.x;
	_data[k++] = _scissors.y;
	_data[k++] = _scissors.z;
	_data[k++] = _scissors.w;

	//bottom right
	_data[k++] = max.x;
	_data[k++] = min.y;

	_data[k++] = u2;
	_data[k++] = v2;
	_data[k++] = (float)_letter->texIndex;

	_data[k++] = fCol;

	_data[k++] = _scissors.x;
	_data[k++] = _scissors.y;
	_data[k++] = _scissors.z;
	_data[k++] = _scissors.w;

	//top left
	_data[k++] = min.x;
	_data[k++] = max.y;

	_data[k++] = u1;
	_data[k++] = v1;
	_data[k++] = (float)_letter->texIndex;

	_data[k++] = fCol;

	_data[k++] = _scissors.x;
	_data[k++] = _scissors.y;
	_data[k++] = _scissors.z;
	_data[k++] = _scissors.w;

	//top right
	_data[k++] = max.x;
	_data[k++] = max.y;

	_data[k++] = u2;
	_data[k++] = v1;
	_data[k++] = (float)_letter->texIndex;

	_data[k++] = fCol;

	_data[k++] = _scissors.x;
	_data[k++] = _scissors.y;
	_data[k++] = _scissors.z;
	_data[k++] = _scissors.w;
}

void Text::TextBlock::reBuild(float* _data, const Vec4& _scissors) {

	float* dataCache = _data;

	uint i = 0;
	uint offset = 4 * SpriteBatch::VERTEXSIZE;
	for (auto line : lines) {

		for (auto letter : line->letters) {

			//whitespace
			if (letter->letter == 32)
				continue;

			addQuad(dataCache + i * offset, i, letter, 0.f, letter->fontColor, _scissors);
			++i;

			if (letter->outlineThickness != 0.f) {
				addQuad(dataCache + i * offset, i, letter, letter->outlineThickness, letter->outlineColor, _scissors);
				++i;
			}
		}

	}
}

void Text::TextBlock::calculateBounds() {
	//calculate bounds
	aabb->clr();
	for (auto l : lines) {
		l->calculateBounds(pos, aabb);
	}
}

Text::Style& Text::TextBlock::getStyle() {
	jobs.push(new Job(TextJob::t_rebuild));
	return defaultStyle;
}

//---------------------- Line ----------------------\\

Text::Line::Line() {
	globalBounds = new BoundingBox();
	localBounds = new BoundingBox();
}

void Text::Line::reset() {
	localPos = Vec2(0, 0);
	globalPos = Vec2(0, 0);
	globalBounds->clr();
	localBounds->clr();
	spacing = std::numeric_limits<float>::infinity();
	maxWidth = 0;
	for (auto l : letters)
		l->free();
	letters.clear();
}

void Text::Line::calculateBounds(const Vec2& _pos, BoundingBox* _aabb) {

	globalPos = localPos + _pos;

	BoundingBox tbounds;
	for (auto l : letters)
		l->calculateBounds(globalPos, &tbounds);

	localBounds->clr()->ext(&tbounds);
	globalBounds->clr()->ext(localBounds);
	globalBounds->min += Vec4(globalPos, 0.f, 1.f);
	globalBounds->max += Vec4(globalPos, 0.f, 1.f);

	_aabb->ext(localBounds);

}

Text::Line* Text::Line::get() {
	if (cache.empty())
		return new Line();
	else {
		auto l = cache.front();
		cache.pop();
		return l;
	}
}

void Text::Line::free() {
	reset();
	cache.push(this);
}

Text::Letter* Text::Line::isOver(const Vec2& _pos) {
	if(!globalBounds->contains(Vec4(_pos.x, _pos.y, 0.f, 1.f))) return nullptr;
	else {
		for (auto l : letters)
			if (l->isOver(_pos))
				return l;
		return nullptr;
	}
}

//---------------------- Letter ----------------------\\

void Text::Letter::reset() {
	bold = false;
	italic = false;
	localPos = Vec2(0, 0);
	globalPos = Vec2(0, 0);
	localBounds->clr();
	globalBounds->clr();
	letter = 32;
	font = nullptr;
	fontSize = 18;
	fontColor = sf::Color::White;
	outlineThickness = 0;
	outlineColor = sf::Color::White;
	texIndex = 0;
}

void Text::Letter::calculateBounds(const Vec2& _pos, BoundingBox* _aabb) {

	globalPos = localPos + _pos;

	globalBounds->set(localBounds);
	globalBounds->min += Vec4(globalPos, 0.f, 1.f);
	globalBounds->max += Vec4(globalPos, 0.f, 1.f);

	_aabb->ext(localBounds);
}

void Text::Letter::copyStyle(Style& _s) {
	font = _s.font;
	fontColor = _s.fontColor;
	outlineColor = _s.outlineColor;
	fontSize = _s.fontSize;
	outlineThickness = _s.outlineThickness;
	bold = _s.bold;
}

Text::Letter* Text::Letter::get() {
	if (cache.empty())
		return new Letter();
	else {
		auto l = cache.front();
		cache.pop();
		return l;
	}
}

void Text::Letter::free() {
	reset();
	cache.push(this);
}

bool Text::Letter::isOver(const Vec2& _pos) {
	return globalBounds->contains(_pos.x, _pos.y, 0.f);
}
