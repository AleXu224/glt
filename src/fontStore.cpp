#include "fontStore.hpp"

#include "atlas.hpp"
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <print>
#include <string>

#include "engine/textQuad.hpp"

#include <freetype/freetype.h>
#include <freetype/fttypes.h>
#include <utf8/cpp17.h>

#include "roboto-bold.hpp"
#include "roboto-bolditalic.hpp"
#include "roboto-italic.hpp"
#include "roboto-regular.hpp"


using namespace squi;

FontProvider FontStore::defaultFont = FontProvider{
	.key = "default",
	.provider = []() {
		return std::vector(Fonts::roboto.begin(), Fonts::roboto.end());
	},
};
FontProvider FontStore::defaultFontBold = FontProvider{
	.key = "defaultBold",
	.provider = []() {
		return std::vector(Fonts::robotoBold.begin(), Fonts::robotoBold.end());
	},
};
FontProvider FontStore::defaultFontItalic = FontProvider{
	.key = "defaultItalic",
	.provider = []() {
		return std::vector(Fonts::robotoItalic.begin(), Fonts::robotoItalic.end());
	},
};
FontProvider FontStore::defaultFontBoldItalic = FontProvider{
	.key = "defaultBoldItalic",
	.provider = []() {
		return std::vector(Fonts::robotoBoldItalic.begin(), Fonts::robotoBoldItalic.end());
	},
};

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

std::shared_ptr<FontStore::Font> FontStore::getFont(const FontProvider &provider) {
	std::scoped_lock lock{fontsMtx};
	if (auto it = fonts().find(provider.key); it != fonts().end()) {
		if (it->second.expired()) {
			it->second = std::make_shared<FontStore::Font>(provider);
		}
		return it->second.lock();
	}

	auto font = std::make_shared<Font>(provider);
	fonts().insert({provider.key, font});
	return font;
}

std::unordered_map<std::string, std::weak_ptr<FontStore::Font>> &FontStore::fonts() {
	// Make sure device is constructed before the font store
	[[maybe_unused]] static bool pre = []() {
		[[maybe_unused]] auto val = Engine::Vulkan::device();
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

std::tuple<std::vector<std::vector<Engine::TextQuad>>, float, float> FontStore::Font::generateQuads(std::string_view text, float size, const vec2 &pos, const Color &color, std::optional<float> maxWidth) {
	std::lock_guard lock{fontMtx};
	if (!face) return {{}, 0.f, 0.f};
	const int32_t maxWidthClamped = [&]() -> int32_t {
		if (maxWidth.has_value()) {
			return static_cast<int32_t>(std::round(std::max(maxWidth.value(), 0.0f)));
		}
		return std::numeric_limits<int32_t>::max();
	}();
	std::vector<std::vector<Engine::TextQuad>> quads{};
	quads.resize(1, std::vector<Engine::TextQuad>{});
	struct CharData {
		Font::CharInfo &charInfo;
		int32_t offsetX = 0;
		int32_t offsetY = 0;
		char32_t character;
	};
	std::vector<CharData> currentWordCharData{};
	int32_t widestLine = 0;
	int32_t currentLineWidth = 0;
	int32_t currentWordWidth = 0;
	uint32_t previousCharIndex = 0;
	if (!loaded) {
		return {quads, 0, 0};
	}
	FT_Set_Pixel_Sizes(face, 0, static_cast<int32_t>(std::round(std::abs(size))) /* Size needs to be rounded*/);
	const int32_t lineHeight = (face->size->metrics.ascender >> 6) - (face->size->metrics.descender >> 6);

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
					toFloat(currentLineWidth + charData.offsetX) + charData.charInfo.offset.x,
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
	const auto pushWhitespaceToLine = [&]() {
		const auto lines = quads.size();
		const uint32_t yOffset = (lines - 1) * lineHeight;
		const auto toFloat = [](int32_t i) {
			return static_cast<float>(i);
		};
		auto it = std::lower_bound(currentWordCharData.begin(), currentWordCharData.end(), ' ', [&](const CharData &a, char) {
			return a.character == ' ' && (a.offsetX + currentLineWidth + a.charInfo.advance) <= maxWidthClamped;
		});
		int32_t whiteSpaceSize = 0;
		for (const auto &charData: std::span<CharData>(currentWordCharData.begin(), it)) {
			quads.back().emplace_back(Engine::TextQuad::Args{
				.color{color},
				.position{pos},
				.size{charData.charInfo.size},
				.offset{
					toFloat(currentLineWidth + charData.offsetX) + charData.charInfo.offset.x,
					toFloat(static_cast<int32_t>(yOffset) + charData.offsetY) + charData.charInfo.offset.y,
				},
				.uvTopLeft = charData.charInfo.uvTopLeft,
				.uvBottomRight = charData.charInfo.uvBottomRight,
			});
			whiteSpaceSize += charData.charInfo.advance;
		}
		currentWordCharData = std::vector<CharData>(it, currentWordCharData.end());
		for (auto &charData: currentWordCharData) {
			charData.offsetX -= whiteSpaceSize;
		}
		currentLineWidth += whiteSpaceSize;
		currentWordWidth -= whiteSpaceSize;
	};

	std::u32string u32text = utf8::utf8to32(text);
	for (auto character: u32text) {
		auto &charInfo = getCharInfo(character, sizeMap);

		if (currentLineWidth != 0 || currentWordWidth != 0) {
			currentWordWidth += charInfo.getKerning(face, previousCharIndex);

			if (character == ' ') {
				pushWordToLine();
			}
		}

		currentWordCharData.emplace_back(CharData{
			.charInfo = charInfo,
			.offsetX = currentWordWidth,
			.offsetY = face->size->metrics.ascender >> 6,
			.character = character,
		});
		currentWordWidth += charInfo.advance;

		if (currentLineWidth != 0 && (currentLineWidth + currentWordWidth) > maxWidthClamped) {
			pushWhitespaceToLine();
			widestLine = std::max(currentLineWidth, widestLine);
			quads.emplace_back();
			currentLineWidth = 0;
		}

		previousCharIndex = charInfo.index;
	}
	pushWordToLine();

	widestLine = std::max(currentLineWidth, widestLine);

	return {quads, widestLine, quads.size() * lineHeight};
}

std::shared_ptr<Engine::Texture> squi::FontStore::Font::getTexture() const {
	return impl->atlas.getTexture();
}
