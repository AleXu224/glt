#include "textBatch.hpp"
#include "stdexcept"

using namespace squi;

TextBatch::TextBatch(std::string fontPath) {
	this->fontPath = fontPath;
	this->atlas = std::shared_ptr<texture_atlas_t>(texture_atlas_new(512, 512, 1), texture_atlas_delete);
	// const char *cache = " !\"#$%&'()*+,-./0123456789:;<=>?"
	// 					"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
	// 					"`abcdefghijklmnopqrstuvwxyz{|}~";

	this->font = std::shared_ptr<texture_font_t>(texture_font_new_from_file(this->atlas.get(), 32, fontPath.c_str()), texture_font_delete);
	font->rendermode = RENDER_SIGNED_DISTANCE_FIELD;
	// texture_font_load_glyphs(this->font.get(), cache);
	glGenTextures(1, &atlas->id);
	glBindTexture(GL_TEXTURE_2D, atlas->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas->width, atlas->height,
				 0, GL_RED, GL_UNSIGNED_BYTE, atlas->data);


	glGenVertexArrays(1, &this->vao);
	glBindVertexArray(this->vao);
	glGenBuffers(1, &this->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	constexpr auto stride = sizeof(TextVertex);
	uintptr_t offset = 0;
	glBufferData(GL_ARRAY_BUFFER, TEXT_BATCH * 4 * stride, quads.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void *) offset);
	glEnableVertexAttribArray(0);
	offset += sizeof(glm::vec2);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void *) offset);
	glEnableVertexAttribArray(1);
	offset += sizeof(glm::vec2);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *) offset);
	glEnableVertexAttribArray(2);
	offset += sizeof(glm::vec2);

	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride, (void *) offset);
	glEnableVertexAttribArray(3);
	// offset += sizeof(glm::vec4);

	glGenBuffers(1, &this->ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, TEXT_BATCH * 6 * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);
}

void TextBatch::freeBuffers() {
	glDeleteBuffers(1, &this->vbo);
	glDeleteBuffers(1, &this->ebo);
}

unsigned int TextBatch::reserveQuad() {
	if (this->freeQuads.size() > 0) {
		auto id = this->freeQuads.back();
		this->freeQuads.pop_back();
		return id;
	} else {
		auto id = this->firstFreeQuad;
		this->firstFreeQuad++;
		return id;
	}
}

void TextBatch::freeQuad(unsigned int id) {
	if (id == this->firstFreeQuad - 1) {
		this->firstFreeQuad--;
	} else {
		this->freeQuads.push_back(id);
	}
	indices[id * 6 + 0] = 0;
	indices[id * 6 + 1] = 0;
	indices[id * 6 + 2] = 0;
	indices[id * 6 + 3] = 0;
	indices[id * 6 + 4] = 0;
	indices[id * 6 + 5] = 0;
}

std::span<TextVertex> TextBatch::getQuad(unsigned int id) {
	return std::span<TextVertex>(&this->quads[id * 4], 4);
}

std::vector<unsigned int> TextBatch::createQuads(std::string text, glm::vec2 position, float size, glm::vec4 color) {
	std::vector<unsigned int> quadIds{};
	glm::vec2 cursor = position;
	float scale = size / (this->font->ascender);
	cursor.y += this->font->descender * scale;
	auto ascender = this->font->ascender * scale;

	for (auto charIter = text.begin(); charIter != text.end(); charIter++) {
		const auto &character = *charIter;
		auto glyph = texture_font_find_glyph(this->font.get(), &character);
		// If the glyph is not in the atlas then load it and refresh the texture
		if (glyph == nullptr) {
			glyph = texture_font_get_glyph(this->font.get(), &character);

			if (glyph == nullptr) {
				continue;
			}

			// load the new texture atlas
			glBindTexture(GL_TEXTURE_2D, this->atlas->id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, this->atlas->width, this->atlas->height,
						 0, GL_RED, GL_UNSIGNED_BYTE, this->atlas->data);
		}

		const auto quadId = this->reserveQuad();
		auto quad = this->getQuad(quadId);
		float kerning = 0.0f;
		if (charIter != text.begin()) {
			kerning = texture_glyph_get_kerning(glyph, &(*(charIter - 1)));
		}
		cursor.x += kerning * scale;

		auto scaledSize = glm::vec2(glyph->width, glyph->height) * scale;
		auto scaledOffset = glm::vec2(glyph->offset_x, glyph->offset_y) * scale;

		quad[0].position = cursor + glm::vec2(scaledOffset.x, ascender - scaledOffset.y);
		quad[1].position = cursor + glm::vec2(scaledOffset.x + scaledSize.x, ascender - scaledOffset.y);
		quad[2].position = cursor + glm::vec2(scaledOffset.x + scaledSize.x, ascender - scaledOffset.y + scaledSize.y);
		quad[3].position = cursor + glm::vec2(scaledOffset.x, ascender - scaledOffset.y + scaledSize.y);
		quad[0].texCoord = glm::vec2(glyph->s0, glyph->t0);
		quad[1].texCoord = glm::vec2(glyph->s1, glyph->t0);
		quad[2].texCoord = glm::vec2(glyph->s1, glyph->t1);
		quad[3].texCoord = glm::vec2(glyph->s0, glyph->t1);
		quad[0].size = glm::vec2(scaledSize.x, scaledSize.y);
		quad[1].size = glm::vec2(scaledSize.x, scaledSize.y);
		quad[2].size = glm::vec2(scaledSize.x, scaledSize.y);
		quad[3].size = glm::vec2(scaledSize.x, scaledSize.y);
		quad[0].color = color;
		quad[1].color = color;
		quad[2].color = color;
		quad[3].color = color;

		indices[quadId * 6 + 0] = quadId * 4 + 0;
		indices[quadId * 6 + 1] = quadId * 4 + 1;
		indices[quadId * 6 + 2] = quadId * 4 + 2;
		indices[quadId * 6 + 3] = quadId * 4 + 0;
		indices[quadId * 6 + 4] = quadId * 4 + 2;
		indices[quadId * 6 + 5] = quadId * 4 + 3;

		cursor.x += glyph->advance_x * scale;

		quadIds.push_back(quadId);

		// handle variable length encoding
		if ((unsigned char)character >= 0b11110000) {
			charIter += 3;
		} else if ((unsigned char)character >= 0b11100000) {
			charIter += 2;
		} else if ((unsigned char)character >= 0b11000000) {
			charIter += 1;
		}
	}

	return quadIds;
}

void TextBatch::render() {
	glBindVertexArray(this->vao);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, this->firstFreeQuad * 4 * sizeof(TextVertex), this->quads.data());

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, this->firstFreeQuad * 6 * sizeof(unsigned int), this->indices.data());

	glBindTexture(GL_TEXTURE_2D, this->font->atlas->id);


	glDrawElements(GL_TRIANGLES, TEXT_BATCH * 6, GL_UNSIGNED_INT, nullptr);
}
