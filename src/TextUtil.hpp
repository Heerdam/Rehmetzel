#pragma once

#include "MainStruct.hpp"

namespace Heerbann {

	namespace Text {

		

		enum Align {
			left, centre, right
		};

		enum TextDecoration {
			bold, italic, underline, underline2, strikeThrough
		};

		struct Style {
			Style();
			Style(Style&);
			Align align = Align::left;
			sf::Font* font;
			sf::Color fontColor = sf::Color::White;
			sf::Color outlineColor = sf::Color(71, 71, 71, 255);
			unsigned int fontSize = 18;
			float outlineThickness = 0.f;
			bool bold = false;
		};

		//---------------------- FontCache ----------------------\\

		class FontCache {

			friend TextBlock;

			std::unordered_map<std::wstring, TextBlock*> dynamicBlocks;

			std::unordered_map<std::wstring, sf::Font*> fonts;

			static std::queue<TextBlock*> blockCache;

			void free(TextBlock*);

		public:

			static TextBlock* create(std::wstring);

			FontCache();

			void addFont(std::wstring, sf::Font*);

			Style defaultStyle;

		};

		struct BaseLetter {
			enum LetterType { letter, lineBreak };

			LetterType type = letter;
		};

		struct LineBreak : public BaseLetter {
		public:
			LineBreak() {
				type = lineBreak;
			};
		};

		//---------------------- Letter ----------------------\\

		struct Letter : public BaseLetter {

			friend Line;
			friend TextBlock;

		private:
			static std::queue<Letter*> cache;

		public:

			bool bold = false;
			bool italic = false;

			Vec2 localPos; //relative to line
			Vec2 globalPos;

			BoundingBox* localBounds;
			BoundingBox* globalBounds;

			uint letter = 32;

			unsigned int texIndex = 0;

			sf::Font* font = nullptr;

			unsigned int fontSize = 18;
			sf::Color fontColor = sf::Color::White;

			float outlineThickness = 0; //outline thickness
			sf::Color outlineColor = sf::Color::White;

		private:
			Letter() {};
			void reset();
			void calculateBounds(const Vec2&, BoundingBox*);
			void copyStyle(Style&);

		public:
			static Text::Letter* get();
			void free();
			bool isOver(const Vec2&);
		};

		//---------------------- Line ----------------------\\

		struct Line {

			friend TextBlock;

		private:
			static std::queue<Line*> cache;

			TextBlock* parent;

			Vec2 localPos; //relative to block
			Vec2 globalPos;

			BoundingBox* globalBounds;
			BoundingBox* localBounds;

		public:
			std::vector<Letter*> letters;

			float spacing = 0;
			float maxWidth = 0;

		private:
			Line();

			void reset();
			void calculateBounds(const Vec2&, BoundingBox*);
			static Line* get();

		public:
			void free();
			Letter* isOver(const Vec2&);
		};

		//---------------------- Textblock ----------------------\\

		class TextBlock {

			friend FontCache;

			enum TextJob {
				t_rebuild, t_insert, t_update
			};

			struct Job {
				TextJob jobType;
				Job(TextJob _type) : jobType(_type) {};
			};

		private:
			std::queue<Job*> jobs;
			std::vector<Line*> lines;

			std::wstring id;

			Style defaultStyle;
			Align align = Align::left;

			Vec2 pos; //top left corner

			float maxWidth = 250.f;

			BoundingBox* aabb;

			int spriteCount = 0;

			std::wstring currentText;

			Text::FontCache* parent;

		private:
			TextBlock(Text::FontCache*);
			TextBlock(Text::FontCache*, Style);

			void reset();

			void calculateBounds();

			//complete layout
			int layout(std::wstring);

			void addQuad(float*, int, Text::Letter*, float, sf::Color, const Vec4&);

			//complete rebuild
			void reBuild(float*, const Vec4&);

		public:
			void free();
			Style& getStyle();

			/*
			allowed markups:
			font: {fn=font name}
			color: {fc=rrrgggbbbaaa}
			border color: {bc=rrrgggbbbaaa}
			border thickness: {bt=float}
			size: {sz=uint}
			bold: {bl=true}
			italic: {it=true} TODO
			combination: {fn=font name,fc=rrrgggbbbaaa,sz=uint}
			end of style: {end}
			rebuilds the complete cache
			*/
			void setText(std::wstring);

			//updates a part (same amount of letters)
			void update(std::wstring, int);

			//triggers a complete rebuild of the cache
			void setAlign(Align);

			void setWidth(float);

			int layoutAndRebuild(float*, const Vec4&);

			inline float getWidth() {
				return maxWidth;
			}

			//thread safe. only called by spritebatch
			void draw(int&, float*, const Vec4&);

			BoundingBox* bounds();

		};

		//---------------------- SplitFunctor ----------------------\\

		class SplitFunctor {
		public:
			SplitFunctor(std::wregex _splitter, std::wstring _original)
				: splitter{ std::move(_splitter) }
				, original{ std::move(_original) }
			{};

			auto begin() const {
				return std::wsregex_token_iterator{ original.begin(), original.end(), splitter, -1 };
			};

			auto end() const {
				return std::wsregex_token_iterator{};
			};

			template <typename Container>
			operator Container () const {
				return { begin(), end() };
			};

		private:
			std::wregex splitter;
			std::wstring original;
		};

	}
}