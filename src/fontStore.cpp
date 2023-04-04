#include "fontStore.hpp"

#include <utility>

using namespace squi;

static bool isInitialized{false};
FT_Library FontStore::ftLibrary{};

FontStore::Font::Font(std::string fontPath)
	: fontPath(std::move(fontPath)) {
	if (FT_New_Face(ftLibrary, this->fontPath.c_str(), 0, &face)) {
		printf("Failed to load font: %s\n", this->fontPath.c_str());
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

Quad FontStore::Font::getQuad(unsigned char *character, float size) {
	// UTF8 to UTF32
	char32_t codepoint = UTF8ToUTF32(character);

	// Load the character
	FT_Set_Pixel_Sizes(face, 0, static_cast<uint32_t>(std::round(std::abs(size))) /* Size needs to be rounded*/);
	if (FT_Load_Glyph(face, FT_Get_Char_Index(face, codepoint), FT_LOAD_RENDER)) {
		printf("Failed to load glyph: %c (%d)\n", codepoint, codepoint);
		return Quad({Quad::Args{}}); // Return empty quad
	}

	// Check if the character is already in the atlas
	if (chars.contains(size) && chars.at(size).contains(codepoint)) {
		return chars.at(size).at(codepoint).getQuad(atlas.textureView);
	}

	if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
		printf("Failed to render glyph: %c (%d)\n", codepoint, codepoint);
		return Quad({Quad::Args{}});
	}

	// Add the character to the atlas
	auto [uvTopLeft, uvBottomRight, success] = atlas.add(face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer);
	if (!success) {
		// TODO: Add a way to have multiple atlases
		printf("Failed to add glyph to atlas: %c (%d)\n", codepoint, codepoint);
		return Quad({Quad::Args{}});
	}

	// Add the character to the chars map
	chars[size][codepoint] = {
		.uvTopLeft = uvTopLeft,
		.uvBottomRight = uvBottomRight,
		.size = {
			static_cast<float>(face->glyph->bitmap.width),
			static_cast<float>(face->glyph->bitmap.rows),
		},
	};

	return chars.at(size).at(codepoint).getQuad(atlas.textureView);
}

std::unordered_map<std::string, FontStore::Font> FontStore::fonts{};

std::vector<Quad> FontStore::generateQuads(std::string text, const std::string &fontPath, float size, const vec2 &pos, const Color &color) {
	if (!isInitialized) {
		if (FT_Init_FreeType(&ftLibrary)) {
			printf("Failed to initialize FreeType\n");
			exit(1);
		}
		isInitialized = true;
	}

	std::vector<Quad> quads{};
	vec2 cursor{0, 0};// This will be used as offset for each character
	if (!fonts.contains(fontPath)) {
		fonts.insert({fontPath, Font(fontPath)});
	}
	auto &font = fonts.at(fontPath);

	if (!font.loaded) {
		return quads;
	}

	char *previousChar = nullptr;
	for (auto charIter = text.begin(); charIter != text.end(); charIter++) {
		const auto &character = text.at(charIter - text.begin());
		auto quad = font.getQuad((unsigned char *) &character, size);

		FT_Vector kerning{};
		if (previousChar != nullptr) {
			auto index1 = FT_Get_Char_Index(font.face, UTF8ToUTF32((unsigned char *) previousChar));
			auto index2 = FT_Get_Char_Index(font.face, UTF8ToUTF32((unsigned char *) &character));
			FT_Get_Kerning(font.face, index1, index2, FT_KERNING_DEFAULT, &kerning);
		}
		cursor.x += static_cast<float>(kerning.x >> 6);
		// Getting the Y position:
		// FreeType assumes bottom-left as origin while we use top-left
		// So in that case we can just subtract the height of the glyph from the ascender
        cursor.y = static_cast<float>(font.face->size->metrics.ascender >> 6);
//		cursor.y -= static_cast<float>(font.face->glyph->bitmap.rows);

		quad.setPos(pos);
		const vec2 offset = {
			static_cast<float>(font.face->glyph->metrics.horiBearingX >> 6),
			-static_cast<float>(font.face->glyph->metrics.horiBearingY >> 6),
		};
		quad.setOffset(cursor + offset);
		quad.setColor(color);
		quads.emplace_back(std::move(quad));

        cursor.x += static_cast<float>(font.face->glyph->advance.x >> 6);

		previousChar = &text.at(charIter - text.begin());

		if ((unsigned char) character >= 0b11110000) {
			charIter += 3;
		} else if ((unsigned char) character >= 0b11100000) {
			charIter += 2;
		} else if ((unsigned char) character >= 0b11000000) {
			charIter += 1;
		}
	}

//	font.printAtlas();
//
//	exit(1);

	font.updateTexture();

    return quads;
}

void FontStore::Font::updateTexture() {
	atlas.updateTexture();
}