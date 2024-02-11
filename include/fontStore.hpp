#pragma once

#include "engine/instance.hpp"
#include "engine/samplerUniform.hpp"
#include "engine/textQuad.hpp"
#include <freetype/fttypes.h>
#include <optional>
#include <span>
#include <tuple>
#include FT_FREETYPE_H
#include "atlas.hpp"
#include "color.hpp"
#include "unordered_map"

namespace squi {
	class FontStore {
	public:
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

		private:
			Atlas atlas;
			//
			std::unordered_map<float, std::unordered_map<char32_t, CharInfo>> chars{};

			bool loaded{true};
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
			explicit Font(std::string_view fontPath, Engine::Instance &instance);
			explicit Font(std::span<char> fontData, Engine::Instance &instance);

			[[nodiscard]] uint32_t getLineHeight(float size);
			[[nodiscard]] std::tuple<uint32_t, uint32_t> getTextSizeSafe(std::string_view text, float size, std::optional<float> maxWidth = {});
			[[nodiscard]] std::tuple<std::vector<std::vector<Engine::TextQuad>>, float, float> generateQuads(std::string_view text, float size, const vec2 &pos, const Color &color, std::optional<float> maxWidth = {});
			[[nodiscard]] Engine::SamplerUniform &getSampler();
		};

		static std::shared_ptr<Font> getFont(std::string_view fontPath, Engine::Instance &instance);
		static std::optional<std::shared_ptr<Font>> getFontOptional(std::string_view fontPath);

		static bool init;
		static FT_Library ftLibrary;
		static std::unordered_map<std::string, std::weak_ptr<Font>> fonts;
	};
}// namespace squi