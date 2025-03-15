#pragma once

#include "memory"
#include "mutex"
#include "string"
#include "unordered_map"

#include "image/provider.hpp"


namespace squi::Store {
	struct Texture {
		[[nodiscard]] static std::shared_ptr<Engine::Texture> getTexture(const ImageProvider &provider) {
			auto &self = _getInstance();

			// FIXME: make this async
			std::scoped_lock lock{self._dataMtx};

			auto createTexture = [&]() {
				return provider.provider().createTexture();
			};

			if (auto it = self._data.find(provider.key); it != self._data.end()) {
				auto val = it->second.lock();
				if (!val) {
					val = createTexture();
					it->second = val;
				}
				return val;
			}

			auto ret = createTexture();
			self._data.insert({provider.key, ret});
			return ret;
		}

	private:
		static inline Store::Texture &_getInstance() {
			static Store::Texture _{};
			return _;
		}

		std::mutex _dataMtx{};
		std::unordered_map<std::string, std::weak_ptr<Engine::Texture>> _data{};
	};
}// namespace squi::Store