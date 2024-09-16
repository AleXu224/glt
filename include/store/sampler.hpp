#pragma once

#include "memory"
#include "mutex"
#include "unordered_map"

#include "instance.hpp"
#include "samplerUniform.hpp"


namespace squi::Store {
	struct Sampler {
		[[nodiscard]] std::shared_ptr<Engine::SamplerUniform> getSampler(Engine::Instance &instance, std::shared_ptr<Engine::Texture> texture) {
			// FIXME: make this async
			std::scoped_lock lock{_dataMtx};

			auto createSampler = [&]() {
				return std::make_shared<Engine::SamplerUniform>(Engine::SamplerUniform::Args{
					.instance = instance,
					.texture = texture,
				});
			};

			if (auto it = _data.find(&*texture); it != _data.end()) {
				if (it->second.expired()) {
					auto ret = createSampler();
					it->second = ret;
					return ret;
				}
				return it->second.lock();
			}

			auto ret = createSampler();
			_data.insert({&*texture, ret});
			return ret;
		}

	private:
		std::mutex _dataMtx{};
		std::unordered_map<Engine::Texture *, std::weak_ptr<Engine::SamplerUniform>> _data{};
	};
}// namespace squi::Store