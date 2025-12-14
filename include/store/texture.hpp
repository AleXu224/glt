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

			{
				std::scoped_lock lock{self._dataMtx};
				if (auto it = self._data.find(provider.key); it != self._data.end()) {
					if (auto val = it->second.lock()) {
						return val;
					}
				}
			}

			auto ret = provider.provider().createTexture();

			{
				std::scoped_lock lock{self._dataMtx};
				if (auto it = self._data.find(provider.key); it != self._data.end()) {
					if (auto val = it->second.lock()) {
						return val;
					}
					it->second = ret;
					return ret;
				}

				self._data.insert({provider.key, ret});
				return ret;
			}
		}

	private:
		static Store::Texture &_getInstance() {
			static Store::Texture _{};
			return _;
		}

		std::mutex _dataMtx{};
		std::unordered_map<std::string, std::weak_ptr<Engine::Texture>> _data{};
	};
}// namespace squi::Store