#include "fontStore.hpp"

#include "atlas.hpp"
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <print>
#include <span>
#include <string>

#include "engine/textQuad.hpp"

#include "utf8/cpp20.h"// IWYU pragma: keep
#include <freetype/freetype.h>
#include <freetype/fttypes.h>


using namespace squi;

struct squi::FontStore::Font::Impl {
	Atlas atlas;
};

FT_Library &FontStore::ftLibrary() {
	static FT_Library _{};
	[[maybe_unused]] static bool init = []() {
		if (FT_Init_FreeType(&_)) {
			std::println("Failed to initialize FreeType");
			exit(1);
		}
		return true;
	}();
	return _;
}

FontStore::Font::Font(const FontProvider &provider)
	: impl(std::make_unique<Impl>(std::string_view(provider.key))),
	  fontData(provider.provider()) {
	if (FT_New_Memory_Face(ftLibrary(), reinterpret_cast<const FT_Byte *>(fontData.data()), static_cast<FT_Long>(fontData.size()), 0, &face)) {
		std::println("Failed to load font from memory");
		loaded = false;
	}
}

squi::FontStore::Font::~Font() {
	FT_Done_Face(face);
}

std::shared_ptr<FontStore::Font> FontStore::getFont(const FontProvider &provider) {
	std::scoped_lock lock{fontsMtx};
	if (auto it = fonts().find(provider.key); it != fonts().end()) {
		auto font = it->second.lock();
		if (!font) {
			font = std::make_shared<FontStore::Font>(provider);
			it->second = font;
		}
		return font;
	}

	auto font = std::make_shared<Font>(provider);
	fonts().insert({provider.key, font});
	return font;
}

std::unordered_map<std::string, std::weak_ptr<FontStore::Font>> &FontStore::fonts() {
	// Make sure device is constructed before the font store
	[[maybe_unused]] static bool pre = []() {
		[[maybe_unused]] auto &val = glt::Engine::Vulkan::device();
		return true;
	}();

	static std::unordered_map<std::string, std::weak_ptr<FontStore::Font>> _{};
	return _;
}

bool FontStore::Font::generateTexture(char32_t character, float size) {
	return generateTexture(character, chars[size]);
}

bool FontStore::Font::generateTexture(char32_t character, std::unordered_map<char32_t, CharInfo> &sizeMap) {
	// UTF8 to UTF32
	// char32_t codepoint = UTF8ToUTF32(character);

	// Check if the character is already in the atlas
	if (sizeMap.contains(character)) {
		return true;
	}

	// Load the character
	if (FT_Load_Glyph(face, FT_Get_Char_Index(face, character), FT_LOAD_RENDER)) {
		std::println("Failed to load glyph: ({:#08x})", static_cast<uint32_t>(character));
		return false;
	}

	if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
		std::println("Failed to render glyph: ({:#08x})", static_cast<uint32_t>(character));
		return false;
	}

	// Add the character to the atlas
	auto [uvTopLeft, uvBottomRight, success] = impl->atlas.add(face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer);
	if (!success) {
		// TODO: Add a way to have multiple atlases
		std::println("Failed to add glyph to atlas: ({:#08x})", static_cast<uint32_t>(character));
		return false;
	}

	// Add the character to the chars map
	sizeMap[character] = {
		.uvTopLeft = uvTopLeft,
		.uvBottomRight = uvBottomRight,
		.size = {
			static_cast<float>(face->glyph->bitmap.width),
			static_cast<float>(face->glyph->bitmap.rows),
		},
		.offset = {
			static_cast<float>(face->glyph->metrics.horiBearingX >> 6),
			-static_cast<float>(face->glyph->metrics.horiBearingY >> 6),
		},
		.advance = face->glyph->metrics.horiAdvance >> 6,
		.index = FT_Get_Char_Index(face, character),
	};

	return true;
}

FontStore::Font::CharInfo &FontStore::Font::getCharInfo(char32_t character, float size) {
	if (!generateTexture(character, size)) {
		return chars.at(size).at(0);
	}

	// UTF8 to UTF32
	// const char32_t codepoint = UTF8ToUTF32(character);
	return chars.at(size).at(character);
}

FontStore::Font::CharInfo &FontStore::Font::getCharInfo(char32_t character, std::unordered_map<char32_t, CharInfo> &sizeMap) {
	// const char32_t codepoint = UTF8ToUTF32(character);
	if (auto it = sizeMap.find(character); it != sizeMap.end()) {
		return it->second;
	}

	if (!generateTexture(character, sizeMap)) {
		return sizeMap.at(0);
	}

	// UTF8 to UTF32
	return sizeMap.at(character);
}

std::unordered_map<char32_t, FontStore::Font::CharInfo> &FontStore::Font::getSizeMap(float size) {
	return chars[size];
}

uint32_t FontStore::Font::getLineHeight(float size) {
	if (!face) return 0;
	FT_Set_Pixel_Sizes(face, 0, static_cast<uint32_t>(size));

	return (face->size->metrics.ascender >> 6) - (face->size->metrics.descender >> 6);
}

std::tuple<uint32_t, uint32_t> FontStore::Font::getTextSizeSafe(std::string_view text, float size, std::optional<float> maxWidth) {
	std::lock_guard lock{fontMtx};
	if (!face) return {0, 0};
	const int32_t maxWidthClamped = [&]() -> int32_t {
		if (maxWidth.has_value()) {
			return static_cast<int32_t>(std::round(std::max(maxWidth.value(), 0.0f)));
		}
		return std::numeric_limits<int32_t>::max();
	}();
	FT_Set_Pixel_Sizes(face, 0, static_cast<uint32_t>(std::round(std::abs(size))) /* Size needs to be rounded*/);

	auto &sizeMap = getSizeMap(size);
	uint32_t prevCharIndex = 0;
	const uint32_t lineHeight = (face->size->metrics.ascender >> 6) - (face->size->metrics.descender >> 6);

	int32_t widestLine = 0;
	int32_t currentLineWidth = 0;
	int32_t currentWordWhitespace = 0;
	int32_t currentWordWidth = 0;
	uint32_t lineCount = 1;

	std::u32string u32text = utf8::utf8to32(text);
	for (auto character: u32text) {
		auto &charInfo = getCharInfo(character, sizeMap);

		if (character == '\n') {
			widestLine = std::max(currentLineWidth + currentWordWidth, widestLine);
			currentLineWidth = 0;
			currentWordWidth = 0;
			currentWordWhitespace = 0;
			++lineCount;
			prevCharIndex = 0;
			continue;
		}
		if (character == '\r') continue;

		if (currentLineWidth != 0 || currentWordWidth != 0) {
			currentWordWidth += charInfo.getKerning(face, prevCharIndex);

			if (character == ' ') {
				currentLineWidth += currentWordWidth;
				currentWordWidth = 0;
				currentWordWhitespace = 0;
			}
		}

		currentWordWidth += charInfo.advance;
		if (character == ' ') currentWordWhitespace += charInfo.advance;

		if (currentLineWidth != 0 && (currentLineWidth + currentWordWidth) > maxWidthClamped) {
			if (character == ' ') {
				// If by the end of the row there is only a series of spaces then get the last space to the new line
				currentLineWidth += currentWordWidth - charInfo.advance;
				currentWordWidth = charInfo.advance;
			} else {
				// If there is a word at the end of the row then leave the space at the front on this row and advance the word
				currentLineWidth += currentWordWhitespace;
				currentWordWidth -= currentWordWhitespace;
			}
			widestLine = std::max(currentLineWidth, widestLine);
			currentWordWhitespace = 0;
			currentLineWidth = 0;
			++lineCount;
		}

		prevCharIndex = charInfo.index;
	}

	if (currentWordWidth == 0 && currentLineWidth == 0) lineCount--;

	widestLine = std::max(currentLineWidth + currentWordWidth, widestLine);

	return {widestLine, lineCount * lineHeight};
}

TextLayout FontStore::Font::textLayout(std::string_view text, float size, std::optional<float> maxWidth) {
	std::lock_guard lock{fontMtx};
	TextLayout result{};
	if (!face || !loaded) return result;

	const int32_t maxWidthClamped = [&]() -> int32_t {
		if (maxWidth.has_value()) {
			return static_cast<int32_t>(std::round(std::max(maxWidth.value(), 0.0f)));
		}
		return std::numeric_limits<int32_t>::max();
	}();
	FT_Set_Pixel_Sizes(face, 0, static_cast<int32_t>(std::round(std::abs(size))));
	const int32_t lineHeight = (face->size->metrics.ascender >> 6) - (face->size->metrics.descender >> 6);
	result.lineHeight = static_cast<float>(lineHeight);

	auto &sizeMap = getSizeMap(size);

	result.quads.resize(1);

	struct QuadChar {
		Font::CharInfo &charInfo;
		int32_t offsetX = 0;
		int32_t offsetY = 0;
		char32_t character;
		int64_t byteOffset = 0;
	};
	std::vector<QuadChar> currentWordChars{};

	int32_t widestLine = 0;
	int32_t currentLineWidth = 0;
	int32_t currentWordWidth = 0;
	uint32_t previousCharIndex = 0;
	uint32_t currentLineIndex = 0;

	const auto toFloat = [](int32_t i) {
		return static_cast<float>(i);
	};

	const auto pushWordToLine = [&]() {
		const uint32_t yOffset = currentLineIndex * lineHeight;
		for (const auto &qc: currentWordChars) {
			result.glyphs.push_back({
				.byteOffset = qc.byteOffset,
				.x = toFloat(currentLineWidth + qc.offsetX),
				.advance = toFloat(qc.charInfo.advance),
				.lineIndex = currentLineIndex,
			});
			result.quads.back().emplace_back(glt::Engine::TextQuad::Args{
				.size{qc.charInfo.size},
				.offset{
					toFloat(currentLineWidth + qc.offsetX) + qc.charInfo.offset.x,
					toFloat(static_cast<int32_t>(yOffset) + qc.offsetY) + qc.charInfo.offset.y,
				},
				.uvTopLeft = qc.charInfo.uvTopLeft,
				.uvBottomRight = qc.charInfo.uvBottomRight,
			});
		}
		currentWordChars.clear();

		currentLineWidth += currentWordWidth;
		currentWordWidth = 0;
	};
	const auto pushWhitespaceToLine = [&]() {
		const uint32_t yOffset = currentLineIndex * lineHeight;
		auto it = std::lower_bound(currentWordChars.begin(), currentWordChars.end(), ' ', [&](const QuadChar &a, char) {
			return a.character == ' ' && (a.offsetX + currentLineWidth + a.charInfo.advance) <= maxWidthClamped;
		});
		int32_t whiteSpaceSize = 0;
		for (const auto &qc: std::span(currentWordChars.begin(), it)) {
			result.glyphs.push_back({
				.byteOffset = qc.byteOffset,
				.x = toFloat(currentLineWidth + qc.offsetX),
				.advance = toFloat(qc.charInfo.advance),
				.lineIndex = currentLineIndex,
			});
			result.quads.back().emplace_back(glt::Engine::TextQuad::Args{
				.size{qc.charInfo.size},
				.offset{
					toFloat(currentLineWidth + qc.offsetX) + qc.charInfo.offset.x,
					toFloat(static_cast<int32_t>(yOffset) + qc.offsetY) + qc.charInfo.offset.y,
				},
				.uvTopLeft = qc.charInfo.uvTopLeft,
				.uvBottomRight = qc.charInfo.uvBottomRight,
			});
			whiteSpaceSize += qc.charInfo.advance;
		}
		currentWordChars = std::vector<QuadChar>(it, currentWordChars.end());
		for (auto &qc: currentWordChars) {
			qc.offsetX -= whiteSpaceSize;
		}
		currentLineWidth += whiteSpaceSize;
		currentWordWidth -= whiteSpaceSize;
	};
	const auto startNewLine = [&](std::optional<int64_t> newLineIndex = std::nullopt) {
		widestLine = std::max(currentLineWidth, widestLine);
		result.quads.emplace_back();
		currentLineWidth = 0;
		currentLineIndex = static_cast<uint32_t>(result.quads.size() - 1);
		if (newLineIndex.has_value()) {
			result.glyphs.emplace_back(TextLayout::Glyph{
				.byteOffset = newLineIndex.value(),
				.x = 0,
				.advance = 0,
				.lineIndex = currentLineIndex,
			});
		}
	};

	auto it = text.begin();
	auto end = text.end();
	int64_t byteOffset = 0;
	while (it != end) {
		auto prevIt = it;
		char32_t character = utf8::next(it, end);
		int64_t charByteOffset = byteOffset;
		byteOffset += std::distance(prevIt, it);

		auto &charInfo = getCharInfo(character, sizeMap);

		if (character == '\n') {
			currentWordChars.emplace_back(QuadChar{
				.charInfo = getCharInfo(' ', sizeMap),
				.offsetX = currentWordWidth,
				.offsetY = face->size->metrics.ascender >> 6,
				.character = ' ',
				.byteOffset = charByteOffset,
			});
			result.newlineOffsets.push_back(charByteOffset + 1);
			pushWordToLine();
			startNewLine(charByteOffset);
			previousCharIndex = 0;
			continue;
		}
		if (character == '\r') continue;

		if (currentLineWidth != 0 || currentWordWidth != 0) {
			currentWordWidth += charInfo.getKerning(face, previousCharIndex);

			if (character == ' ') {
				pushWordToLine();
			}
		}

		currentWordChars.emplace_back(QuadChar{
			.charInfo = charInfo,
			.offsetX = currentWordWidth,
			.offsetY = face->size->metrics.ascender >> 6,
			.character = character,
			.byteOffset = charByteOffset,
		});
		currentWordWidth += charInfo.advance;

		if (currentLineWidth != 0 && (currentLineWidth + currentWordWidth) > maxWidthClamped) {
			pushWhitespaceToLine();
			startNewLine();
		}

		previousCharIndex = charInfo.index;
	}
	pushWordToLine();

	widestLine = std::max(currentLineWidth, widestLine);
	result.widestLine = static_cast<float>(widestLine);
	result.totalHeight = static_cast<float>(result.quads.size() * lineHeight);

	return result;
}

std::tuple<std::vector<std::vector<glt::Engine::TextQuad>>, float, float> FontStore::Font::generateQuads(std::string_view text, float size, const vec2 &pos, const Color &color, std::optional<float> maxWidth) {
	auto layout = textLayout(text, size, maxWidth);
	for (auto &quadVec: layout.quads) {
		for (auto &quad: quadVec) {
			quad.setPos(pos);
			quad.setColor(color);
		}
	}
	return {std::move(layout.quads), layout.widestLine, layout.totalHeight};
}

std::shared_ptr<glt::Engine::Texture> squi::FontStore::Font::getTexture() const {
	return impl->atlas.getTexture();
}

ImageProvider squi::FontStore::Font::getImageProvider() const {
	return impl->atlas.getProvier();
}

void squi::FontStore::Font::writePendingTextures() {
	impl->atlas.writePendingTextures();
}
