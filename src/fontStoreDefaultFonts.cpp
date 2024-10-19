#include "fontStore.hpp"

#include "materialSymbols-200-24-filled.hpp"
#include "materialSymbols-200-24.hpp"
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
FontProvider FontStore::defaultIcons = FontProvider{
	.key = "defaultIcons",
	.provider = []() {
		return std::vector(Fonts::materialSymbols_200_24.begin(), Fonts::materialSymbols_200_24.end());
	},
};
FontProvider FontStore::defaultIconsFilled = FontProvider{
	.key = "defaultIcons",
	.provider = []() {
		return std::vector(Fonts::materialSymbols_200_24_filled.begin(), Fonts::materialSymbols_200_24_filled.end());
	},
};