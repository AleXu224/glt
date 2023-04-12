#include "fontStore.hpp"
#include "texture.hpp"

#include <utility>

using namespace squi;

static bool isInitialized{false};
FT_Library FontStore::ftLibrary{};

FontStore::Font::Font(std::string_view fontPath) {
	const std::string fontPathStr(fontPath);
	if (FT_New_Face(ftLibrary, fontPathStr.c_str(), 0, &face)) {
		printf("Failed to load font: %s\n", fontPathStr.c_str());
		loaded = false;
	}
}

char32_t UTF8ToUTF32(const unsigned char *character) {
	char32_t codepoint{};
	if (character[0] < 0x80) {
		codepoint = character[0];
	} else if (character[0] < 0xE0) {
		codepoint = (character[0] & 0x1F) << 6;
		codepoint |= character[1] & 0x3F;
	} else if (character[0] < 0xF0) {
		codepoint = (character[0] & 0x0F) << 12;
		codepoint |= (character[1] & 0x3F) << 6;
		codepoint |= character[2] & 0x3F;
	} else if (character[0] < 0xF8) {
		codepoint = (character[0] & 0x07) << 18;
		codepoint |= (character[1] & 0x3F) << 12;
		codepoint |= (character[2] & 0x3F) << 6;
		codepoint |= character[3] & 0x3F;
	}
	return codepoint;
}

bool FontStore::Font::generateTexture(unsigned char *character, float size) {
	return generateTexture(character, chars[size]);
}

bool FontStore::Font::generateTexture(unsigned char *character, std::unordered_map<char32_t, CharInfo> &sizeMap) {
	// UTF8 to UTF32
	char32_t codepoint = UTF8ToUTF32(character);

	// Check if the character is already in the atlas
	if (sizeMap.contains(codepoint)) {
		return true;
	}

	// Load the character
	if (FT_Load_Glyph(face, FT_Get_Char_Index(face, codepoint), 0)) {
		printf("Failed to load glyph: %c (%d)\n", codepoint, codepoint);
		return false;
	}

	if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
		printf("Failed to render glyph: %c (%d)\n", codepoint, codepoint);
		return false;
	}

	// Add the character to the atlas
	auto [uvTopLeft, uvBottomRight, success] = atlas.add(face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer);
	if (!success) {
		// TODO: Add a way to have multiple atlases
		printf("Failed to add glyph to atlas: %c (%d)\n", codepoint, codepoint);
		return false;
	}

	// Add the character to the chars map
	sizeMap[codepoint] = {
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
		.advance = static_cast<float>(face->glyph->metrics.horiAdvance >> 6),
		.index = FT_Get_Char_Index(face, codepoint),
	};

	return true;
}

const FontStore::Font::CharInfo &FontStore::Font::getCharInfo(unsigned char *character, float size) {
	if (!generateTexture(character, size)) {
		return chars.at(size).at(0);
	}

	// UTF8 to UTF32
	const char32_t codepoint = UTF8ToUTF32(character);
	return chars.at(size).at(codepoint);
}

const FontStore::Font::CharInfo &FontStore::Font::getCharInfo(unsigned char *character, std::unordered_map<char32_t, CharInfo> &sizeMap) {
	if (!generateTexture(character, sizeMap)) {
		return sizeMap.at(0);
	}

	// UTF8 to UTF32
	const char32_t codepoint = UTF8ToUTF32(character);
	return sizeMap.at(codepoint);
}

Atlas &FontStore::Font::getAtlas() {
	return atlas;
}

std::unordered_map<char32_t, FontStore::Font::CharInfo> &FontStore::Font::getSizeMap(float size) {
	return chars[size];
}

std::unordered_map<std::string_view, FontStore::Font> FontStore::fonts{};

uint32_t FontStore::getLineHeight(std::string_view fontPath, float size) {
	const auto &font = fonts.at(fontPath);

	FT_Set_Pixel_Sizes(font.face, 0, static_cast<uint32_t>(size));

	return (font.face->size->metrics.ascender >> 6) - (font.face->size->metrics.descender >> 6);
}

std::tuple<uint32_t, uint32_t> FontStore::getTextSize(std::string_view text, std::string_view fontPath, float size) {
	uint32_t width = 0;
	auto &font = fonts.at(fontPath);
	auto &sizeMap = font.getSizeMap(size);
	char *prevChar = nullptr;
	const Font::CharInfo *prevCharInfo = nullptr;

	uint8_t skip = 0;

	for (auto charIter = text.begin(); charIter != text.end(); charIter++) {
		if (skip) {
			skip--;
			continue;
		}
		const auto &character = text.at(charIter - text.begin());

		const auto &charInfo = font.getCharInfo((unsigned char *) &character, sizeMap);

		FT_Vector kerning{};
		if (prevChar) {
			FT_Get_Kerning(
				font.face,
				prevCharInfo->index,
				charInfo.index,
				FT_KERNING_DEFAULT,
				&kerning);
		}

		width += static_cast<uint32_t>(charInfo.advance) + static_cast<uint32_t>(charInfo.offset.x) + (kerning.x >> 6);

		if ((unsigned char) character >= 0b11110000) {
			skip = 3;
		} else if ((unsigned char) character >= 0b11100000) {
			skip = 2;
		} else if ((unsigned char) character >= 0b11000000) {
			skip = 1;
		}

		prevChar = (char *) &character;
		prevCharInfo = &charInfo;
	}

	return {width, (font.face->size->metrics.ascender >> 6) - (font.face->size->metrics.descender >> 6)};
}

std::tuple<std::vector<std::vector<Quad>>, float, float> FontStore::generateQuads(std::string_view text, std::string_view fontPath, float size, const vec2 &pos, const Color &color, const float &maxWidth) {
	if (!isInitialized) {
		if (FT_Init_FreeType(&ftLibrary)) {
			printf("Failed to initialize FreeType\n");
			exit(1);
		}
		isInitialized = true;
	}

	std::vector<std::vector<Quad>> quads{};
	quads.resize(1, std::vector<Quad>{});
	int32_t cursorX = 0;
	int32_t cursorY = 0;
	if (!fonts.contains(fontPath)) {
		fonts.insert({fontPath, Font(fontPath)});
	}
	auto &font = fonts.at(fontPath);
	FT_Set_Pixel_Sizes(font.face, 0, static_cast<uint32_t>(std::round(std::abs(size))) /* Size needs to be rounded*/);

	if (!font.loaded) {
		return {quads, 0, 0};
	}

	auto &sizeMap = font.getSizeMap(size);

	const auto &textureView = font.getAtlas().textureView;

	char const *previousChar = nullptr;
	Font::CharInfo const *previousCharInfo = nullptr;
	for (auto charIter = text.begin(); charIter != text.end(); charIter++) {
		const auto &character = text.at(charIter - text.begin());
		const auto &charInfo = font.getCharInfo((unsigned char *) &character, sizeMap);

		FT_Vector kerning{};
		if (previousChar != nullptr) {
			FT_Get_Kerning(font.face, previousCharInfo->index, charInfo.index, FT_KERNING_DEFAULT, &kerning);

			if (*previousChar == ' ' && character != ' ' && maxWidth != -1) {
				auto [width, height] = getTextSize({charIter, std::find(charIter, text.end(), ' ')}, fontPath, size);
				if (cursorX + static_cast<int32_t>(width) > maxWidth) {
					quads.emplace_back();
					cursorX = 0;
					cursorY += height;
				}
			}
		}
		cursorX += kerning.x >> 6;
		// Getting the Y position:
		// FreeType assumes bottom-left as origin while we use top-left
		// So in that case we can just subtract the height of the glyph from the ascender
		const auto savedY = cursorY;
		cursorY += font.face->size->metrics.ascender >> 6;

		if (character != ' ')
			quads.back().emplace_back(Quad::Args{
				.pos{pos},
				.size{charInfo.size},
				.offset{charInfo.offset.withXOffset(cursorX).withYOffset(cursorY)},
				.color{color},
				.texture{textureView},
				.textureType = TextureType::Text,
				.textureUv{
					charInfo.uvTopLeft.x,
					charInfo.uvTopLeft.y,
					charInfo.uvBottomRight.x,
					charInfo.uvBottomRight.y,
				},
			});

		cursorX += charInfo.advance;
		cursorY = savedY;

		previousChar = &text.at(charIter - text.begin());
		previousCharInfo = &charInfo;

		if ((unsigned char) character >= 0b11110000) {
			charIter += 3;
		} else if ((unsigned char) character >= 0b11100000) {
			charIter += 2;
		} else if ((unsigned char) character >= 0b11000000) {
			charIter += 1;
		}
	}

	font.updateTexture();

	return {quads, cursorX, cursorY + (font.face->size->metrics.ascender >> 6) - (font.face->size->metrics.descender >> 6)};
}

void FontStore::Font::updateTexture() {
	atlas.updateTexture();
}