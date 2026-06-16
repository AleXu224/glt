#pragma once

#include "color.hpp"
#include "engine/textQuad.hpp"
#include "image/provider.hpp"
#include "text/provider.hpp"
#include "vec2.hpp"

#include <freetype/fttypes.h>
#include FT_FREETYPE_H

#include "memory"
#include "mutex"
#include "unordered_map"
#include <algorithm>
#include <optional>
#include <tuple>


namespace glt::Engine {
	struct Texture;
}// namespace glt::Engine

namespace squi {
	struct TextLayout {
		struct Glyph {
			int64_t byteOffset{};
			float x{};
			float advance{};
			uint32_t lineIndex{};

			bool isNewline() const {
				return x == 0.f && advance == 0.f;
			}
		};
		std::vector<Glyph> glyphs;
		std::vector<std::vector<glt::Engine::TextQuad>> quads;
		std::vector<int64_t> newlineOffsets;
		float widestLine{};
		float totalHeight{};
		float lineHeight{};

		[[nodiscard]] uint32_t lineForOffset(int64_t byteOffset) const {
			auto it = std::upper_bound(newlineOffsets.begin(), newlineOffsets.end(), byteOffset);
			return static_cast<uint32_t>(std::distance(newlineOffsets.begin(), it));
		}

		[[nodiscard]] float xForOffset(int64_t byteOffset) const {
			if (glyphs.empty()) return 0.f;
			if (byteOffset > 0 && std::binary_search(newlineOffsets.begin(), newlineOffsets.end(), byteOffset))
				return 0.f;
			auto it = std::upper_bound(glyphs.begin(), glyphs.end(), byteOffset, [](int64_t offset, const Glyph &g) {
				return offset < g.byteOffset;
			});
			if (it == glyphs.begin()) return 0.f;
			--it;
			if (it->byteOffset == byteOffset) {
				while (it != glyphs.begin() && (it - 1)->byteOffset == it->byteOffset) {
					--it;
				}
			}
			if (it->byteOffset == byteOffset) return it->x;
			return it->x + it->advance;
		}

		[[nodiscard]] std::optional<size_t> glyphIndexForOffset(int64_t byteOffset) const {
			if (glyphs.empty()) return std::nullopt;
			auto it = std::upper_bound(glyphs.begin(), glyphs.end(), byteOffset, [](int64_t offset, const Glyph &g) {
				return offset < g.byteOffset;
			});
			if (it == glyphs.begin()) return std::nullopt;
			--it;
			if (byteOffset > 0 && std::binary_search(newlineOffsets.begin(), newlineOffsets.end(), byteOffset - 1)) {
				++it;
				if (it == glyphs.end()) return std::nullopt;
			}
			return std::distance(glyphs.begin(), it);
		}
	};

	struct FontStore {
		struct Font {
			struct CharInfo {
				vec2 uvTopLeft{};
				vec2 uvBottomRight{};
				vec2 size{};
				vec2 offset{};
				int32_t advance{};
				FT_UInt index{};

				std::unordered_map<char32_t, int32_t> kerning{};

				int32_t getKerning(const FT_Face &face, const FT_UInt &prevIndex) {
					if (const auto &it = kerning.find(prevIndex); it != kerning.end()) {
						return it->second;
					}

					FT_Vector kerning;
					FT_Get_Kerning(face, prevIndex, index, FT_KERNING_DEFAULT, &kerning);
					this->kerning[prevIndex] = kerning.x >> 6;
					return kerning.x >> 6;
				}
			};

			bool loaded{true};

		private:
			struct Impl;
			std::unique_ptr<Impl> impl;
			//
			std::unordered_map<float, std::unordered_map<char32_t, CharInfo>> chars{};

			std::vector<char> fontData{};
			FT_Face face{};

			/**
			 * @brief Generates the texture for a character and adds it to the atlas. 
			 * There is no additional cost if the texture has already been generated.
			 * 
			 * @param character The character to generate the texture for
			 * @param size The size of the font
			 * @return true 
			 * @return false 
			 */
			bool generateTexture(char32_t character, float size);
			bool generateTexture(char32_t character, std::unordered_map<char32_t, CharInfo> &sizeMap);

			/**
			 * @brief Gets the CharInfo for a character.
			 * 
			 * @param character The character to get the CharInfo for
			 * @param size The size of the font
			 * @return CharInfo& 
			 */
			CharInfo &getCharInfo(char32_t character, float size);
			CharInfo &getCharInfo(char32_t character, std::unordered_map<char32_t, CharInfo> &sizeMap);

			std::unordered_map<char32_t, CharInfo> &getSizeMap(float size);

		public:
			Font(const FontProvider &provider);
			~Font();

			[[nodiscard]] uint32_t getLineHeight(float size);
			[[nodiscard]] std::tuple<uint32_t, uint32_t> getTextSizeSafe(std::string_view text, float size, std::optional<float> maxWidth = {});
			[[nodiscard]] TextLayout textLayout(std::string_view text, float size, std::optional<float> maxWidth = {});
			[[nodiscard]] std::tuple<std::vector<std::vector<glt::Engine::TextQuad>>, float, float> generateQuads(std::string_view text, float size, const vec2 &pos, const Color &color, std::optional<float> maxWidth = {});
			[[nodiscard]] std::shared_ptr<glt::Engine::Texture> getTexture() const;
			[[nodiscard]] ImageProvider getImageProvider() const;
			void writePendingTextures();
			static inline std::mutex fontMtx{};
		};

		static inline std::mutex fontsMtx{};
		static std::shared_ptr<Font> getFont(const FontProvider &provider);

		static FT_Library &ftLibrary();
		static std::unordered_map<std::string, std::weak_ptr<Font>> &fonts();

		static FontProvider defaultFont;
		static FontProvider defaultFontBold;
		static FontProvider defaultFontItalic;
		static FontProvider defaultFontBoldItalic;
		static FontProvider defaultIcons;
		static FontProvider defaultIconsFilled;
	};
}// namespace squi