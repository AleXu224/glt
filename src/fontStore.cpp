#include "atlas.hpp"
#include <cstdint>
#include <freetype/fttypes.h>
#include <memory>
#include <print>

#include "fontStore.hpp"
#include <limits>
#include <optional>
#include <string>
#include <utf8/cpp17.h>


#include <freetype/freetype.h>

using namespace squi;

FT_Library FontStore::ftLibrary{};

bool FontStore::init = []() {
	if (FT_Init_FreeType(&ftLibrary)) {
		std::println("Failed to initialize FreeType");
		exit(1);
	}
	return true;
}();

FontStore::Font::Font(std::string_view fontPath, Engine::Instance &instance) : atlas(instance) {
	const std::string fontPathStr(fontPath);
	if (FT_New_Face(ftLibrary, fontPathStr.c_str(), 0, &face)) {
		std::println("Failed to load font: {}", fontPathStr);
		loaded = false;
	}
}

FontStore::Font::Font(std::span<char> fontData, Engine::Instance &instance) : atlas(instance) {
	if (FT_New_Memory_Face(ftLibrary, reinterpret_cast<const FT_Byte *>(fontData.data()), static_cast<FT_Long>(fontData.size()), 0, &face)) {
		std::println("Failed to load font from memory");
		loaded = false;
	}
}

std::shared_ptr<FontStore::Font> FontStore::getFont(std::string_view fontPath, Engine::Instance &instance) {
	const std::string fontPathStr(fontPath);
	if (fonts.contains(fontPathStr)) {
		if (auto font = fonts[fontPathStr].lock()) {
			return font;
		}
	}
	auto font = std::make_shared<Font>(fontPath, instance);
	fonts[fontPathStr] = font;
	return font;
}

std::optional<std::shared_ptr<FontStore::Font>> squi::FontStore::getFontOptional(std::string_view fontPath) {
	const std::string fontPathStr(fontPath);
	if (fonts.contains(fontPathStr)) {
		if (auto font = fonts[fontPathStr].lock()) {
			return font;
		}
	}
	return {};
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
	auto [uvTopLeft, uvBottomRight, success] = atlas.add(face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer);
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

std::unordered_map<std::string, std::weak_ptr<FontStore::Font>> FontStore::fonts{};

uint32_t FontStore::Font::getLineHeight(float size) {
	FT_Set_Pixel_Sizes(face, 0, static_cast<uint32_t>(size));

	return (face->size->metrics.ascender >> 6) - (face->size->metrics.descender >> 6);
}

std::tuple<uint32_t, uint32_t> FontStore::Font::getTextSizeSafe(std::string_view text, float size, std::optional<float> maxWidth) {
	const uint32_t maxWidthClamped = [&]() -> uint32_t {
		if (maxWidth.has_value()) {
			return static_cast<uint32_t>(std::round(std::max(maxWidth.value(), 0.0f)));
		}
		return std::numeric_limits<uint32_t>::max();
	}();
	FT_Set_Pixel_Sizes(face, 0, static_cast<uint32_t>(std::round(std::abs(size))) /* Size needs to be rounded*/);

	auto &sizeMap = getSizeMap(size);
	uint32_t prevCharIndex = 0;
	const uint32_t lineHeight = (face->size->metrics.ascender >> 6) - (face->size->metrics.descender >> 6);

	uint32_t widestLine = 0;
	uint32_t currentLineWidth = 0;
	uint32_t currentWordWidth = 0;
	uint32_t lineCount = 1;

	std::u32string u32text = utf8::utf8to32(text);
	for (auto character: u32text) {
		auto &charInfo = getCharInfo(character, sizeMap);

		if (currentLineWidth != 0 && (currentLineWidth + currentWordWidth) > static_cast<uint32_t>(maxWidthClamped)) {
			widestLine = std::max(currentLineWidth, widestLine);
			currentLineWidth = 0;
			++lineCount;
		}

		if (currentLineWidth != 0 || currentWordWidth != 0) {
			currentWordWidth += charInfo.getKerning(face, prevCharIndex);

			if (character == ' ') {
				currentLineWidth += currentWordWidth;
				currentWordWidth = 0;
			}
		}

		if (character != ' ') {
			currentWordWidth += charInfo.advance;
		} else if (currentLineWidth != 0) {
			currentLineWidth += charInfo.advance;
		}

		prevCharIndex = charInfo.index;
	}

	widestLine = std::max(currentLineWidth + currentWordWidth, widestLine);

	return {widestLine, lineCount * lineHeight};
}

std::tuple<std::vector<std::vector<Engine::TextQuad>>, float, float> FontStore::Font::generateQuads(std::string_view text, float size, const vec2 &pos, const Color &color, std::optional<float> maxWidth) {
	const uint32_t maxWidthClamped = [&]() -> uint32_t {
		if (maxWidth.has_value()) {
			return static_cast<uint32_t>(std::round(std::max(maxWidth.value(), 0.0f)));
		}
		return std::numeric_limits<uint32_t>::max();
	}();
	std::vector<std::vector<Engine::TextQuad>> quads{};
	quads.resize(1, std::vector<Engine::TextQuad>{});
	struct CharData {
		Font::CharInfo &charInfo;
		int32_t offsetX = 0;
		int32_t offsetY = 0;
	};
	std::vector<CharData> currentWordCharData{};
	uint32_t widestLine = 0;
	uint32_t currentLineWidth = 0;
	uint32_t currentWordWidth = 0;
	uint32_t previousCharIndex = 0;
	if (!loaded) {
		return {quads, 0, 0};
	}
	FT_Set_Pixel_Sizes(face, 0, static_cast<uint32_t>(std::round(std::abs(size))) /* Size needs to be rounded*/);
	const uint32_t lineHeight = (face->size->metrics.ascender >> 6) - (face->size->metrics.descender >> 6);

	auto &sizeMap = getSizeMap(size);

	const auto pushWordToLine = [&]() {
		const auto lines = quads.size();
		const uint32_t yOffset = (lines - 1) * lineHeight;
		const auto toFloat = [](int32_t i) {
			return static_cast<float>(i);
		};
		for (const auto &charData: currentWordCharData) {
			quads.back().emplace_back(Engine::TextQuad::Args{
				.color{color},
				.position{pos},
				.size{charData.charInfo.size},
				.offset{
					toFloat(static_cast<int32_t>(currentLineWidth) + charData.offsetX) + charData.charInfo.offset.x,
					toFloat(static_cast<int32_t>(yOffset) + charData.offsetY) + charData.charInfo.offset.y,
				},
				.uvTopLeft = charData.charInfo.uvTopLeft,
				.uvBottomRight = charData.charInfo.uvBottomRight,
			});
		}
		currentWordCharData.clear();

		currentLineWidth += currentWordWidth;
		currentWordWidth = 0;
	};

	std::u32string u32text = utf8::utf8to32(text);
	for (auto character: u32text) {
		auto &charInfo = getCharInfo(character, sizeMap);

		if (currentLineWidth != 0 && (currentLineWidth + currentWordWidth) > maxWidthClamped) {
			widestLine = std::max(currentLineWidth, widestLine);
			quads.emplace_back();
			currentLineWidth = 0;
		}

		if (currentLineWidth != 0 || currentWordWidth != 0) {
			currentWordWidth += charInfo.getKerning(face, previousCharIndex);

			if (character == ' ') {
				pushWordToLine();
			}
		}

		if (character != ' ') {
			currentWordCharData.emplace_back(CharData{
				.charInfo = charInfo,
				.offsetX = static_cast<int32_t>(currentWordWidth),
				.offsetY = face->size->metrics.ascender >> 6,
			});
			currentWordWidth += charInfo.advance;
		} else if (currentLineWidth != 0) {
			currentLineWidth += charInfo.advance;
		}

		previousCharIndex = charInfo.index;
	}
	pushWordToLine();

	widestLine = std::max(currentLineWidth, widestLine);

	return {quads, widestLine, quads.size() * lineHeight};
}
Engine::SamplerUniform &squi::FontStore::Font::getSampler() {
	return atlas.sampler;
}
