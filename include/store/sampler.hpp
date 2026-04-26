#pragma once

#include "memory"
#include "mutex"
#include "unordered_map"

#include "instance.hpp"
#include "samplerUniform.hpp"


namespace squi::Store {
	struct Sampler {
		[[nodiscard]] std::shared_ptr<glt::Engine::SamplerUniform> getSampler(glt::Engine::Instance &instance, std::shared_ptr<glt::Engine::Texture> texture) {
			// FIXME: make this async
			std::scoped_lock lock{_dataMtx};

			auto createSampler = [&]() {
				return std::make_shared<glt::Engine::SamplerUniform>(glt::Engine::SamplerUniform::Args{
					.instance = instance,
					.texture = texture,
				});
			};

			if (auto it = _data.find(&*texture); it != _data.end()) {
				auto val = it->second.lock();
				if (!val) {
					val = createSampler();
					it->second = val;
				}
				return val;
			}

			auto ret = createSampler();
			_data.insert({&*texture, ret});
			return ret;
		}

	private:
		std::mutex _dataMtx{};
		std::unordered_map<glt::Engine::Texture *, std::weak_ptr<glt::Engine::SamplerUniform>> _data{};
	};
}// namespace squi::Store