#pragma once

#include "atlas.hpp"
#include "color.hpp"
#include "engine/textQuad.hpp"
#include "text/provider.hpp"

#include <freetype/fttypes.h>
#include FT_FREETYPE_H

#include "unordered_map"
#include <optional>
#include <tuple>

#include "roboto-bold.hpp"
#include "roboto-bolditalic.hpp"
#include "roboto-italic.hpp"
#include "roboto-regular.hpp"

namespace squi {
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
			Atlas atlas;
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

			[[nodiscard]] uint32_t getLineHeight(float size);
			[[nodiscard]] std::tuple<uint32_t, uint32_t> getTextSizeSafe(std::string_view text, float size, std::optional<float> maxWidth = {});
			[[nodiscard]] std::tuple<std::vector<std::vector<Engine::TextQuad>>, float, float> generateQuads(std::string_view text, float size, const vec2 &pos, const Color &color, std::optional<float> maxWidth = {});
			[[nodiscard]] std::shared_ptr<Engine::Texture> getTexture() const;
			static inline std::mutex fontMtx{};
		};

		static inline std::mutex fontsMtx{};
		static std::shared_ptr<Font> getFont(const FontProvider &provider);

		static FT_Library &ftLibrary();
		static std::unordered_map<std::string, std::weak_ptr<Font>> &fonts();

		static inline FontProvider defaultFont = FontProvider{
			.key = "default",
			.provider = []() {
				return std::vector(Fonts::roboto.begin(), Fonts::roboto.end());
			},
		};
		static inline FontProvider defaultFontBold = FontProvider{
			.key = "defaultBold",
			.provider = []() {
				return std::vector(Fonts::robotoBold.begin(), Fonts::robotoBold.end());
			},
		};
		static inline FontProvider defaultFontItalic = FontProvider{
			.key = "defaultItalic",
			.provider = []() {
				return std::vector(Fonts::robotoItalic.begin(), Fonts::robotoItalic.end());
			},
		};
		static inline FontProvider defaultFontBoldItalic = FontProvider{
			.key = "defaultBoldItalic",
			.provider = []() {
				return std::vector(Fonts::robotoBoldItalic.begin(), Fonts::robotoBoldItalic.end());
			},
		};
	};
}// namespace squi