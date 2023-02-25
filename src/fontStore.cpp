#include "fontStore.hpp"

using namespace squi;

FontStore::Font::Font(std::string fontPath)
	: fontPath(fontPath),
	  atlas(std::shared_ptr<texture_atlas_t>(texture_atlas_new(1024, 1024, 1), texture_atlas_delete)) {
	// font = std::shared_ptr<texture_font_t>(texture_font_new_from_file(atlas.get(), 16, fontPath.c_str()), texture_font_delete);

	// TODO: Texture might not be cleaned up properly
	glGenTextures(1, &atlas->id);
	glBindTexture(GL_TEXTURE_2D, atlas->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas->width, atlas->height, 0, GL_RED, GL_UNSIGNED_BYTE, atlas->data);
}

std::shared_ptr<texture_font_t> FontStore::Font::getSize(float size) {
	if (!sizes.contains(size)) {
		auto font = std::shared_ptr<texture_font_t>(texture_font_new_from_file(atlas.get(), size, fontPath.c_str()), texture_font_delete);
		font->rendermode = RENDER_NORMAL;
		sizes.insert({size, font});
	}
	return sizes.at(size);
}

std::unordered_map<std::string, FontStore::Font> FontStore::fonts{};

std::shared_ptr<texture_font_t> FontStore::getFont(std::string fontPath, float size) {
	if (!fonts.contains(fontPath)) {
		fonts.insert({fontPath, Font(fontPath)});
	}
	return fonts.at(fontPath).getSize(size);
}

std::vector<Quad> FontStore::generateQuads(std::string text, std::string fontPath, float size, glm::vec2 pos, glm::vec4 color) {
	std::vector<Quad> quads;
	glm::vec2 cursor;// This will be used as offset for each character
	auto font = getFont(fontPath, size);
	cursor.y += font->ascender;

	for (auto charIter = text.begin(); charIter != text.end(); charIter++) {
		const auto &character = text.at(charIter - text.begin());
		auto glyph = texture_font_find_glyph(font.get(), &character);

		if (glyph == nullptr) {
			glyph = texture_font_get_glyph(font.get(), &character);

			if (glyph == nullptr) {
				continue;
			}

			glBindTexture(GL_TEXTURE_2D, font->atlas->id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font->atlas->width, font->atlas->height, 0, GL_RED, GL_UNSIGNED_BYTE, font->atlas->data);
		}

		auto kerning = 0.0f;
		if (charIter != text.begin()) {
			kerning = texture_glyph_get_kerning(glyph, &text.at(charIter - text.begin() - 1));
		}
		cursor.x += kerning;
        cursor.y = font->ascender - glyph->offset_y;

		quads.emplace_back(Quad::Args{
			.pos = pos,
			.size{glyph->width, glyph->height},
			.offset = cursor,
			.color = color,
			.textureId = font->atlas->id,
            .textureType = Quad::TextureType::Text,
			.textureUv = {
				glyph->s0,
				glyph->t0,
				glyph->s1,
				glyph->t1,
			},
		});

        cursor.x += glyph->advance_x;

		if ((unsigned char) character >= 0b11110000) {
			charIter += 3;
		} else if ((unsigned char) character >= 0b11100000) {
			charIter += 2;
		} else if ((unsigned char) character >= 0b11000000) {
			charIter += 1;
		}
	}

    return quads;
}