#pragma once

#include "any"
#include "functional"
#include "memory"
#include "mutex"
#include "unordered_map"


namespace squi::Store {
	template<class T>
	struct PipelineProvider {
		std::string key;
		using Args = T::Args;
		std::function<Args(void)> provider{};
	};

	struct Pipeline {
		template<class T>
		[[nodiscard]] std::shared_ptr<T> getPipeline(const PipelineProvider<T> &provider) {
			// FIXME: make this async
			std::scoped_lock lock{_dataMtx};

			auto createPipeline = [&]() {
				return std::make_shared<T>(provider.provider());
			};

			if (auto it = _data.find(provider.key); it != _data.end()) {
				auto val = std::any_cast<std::weak_ptr<T>>(it->second);
				if (val.expired()) {
					auto ret = createPipeline();
					val = ret;
					return ret;
				}
				return val.lock();
			}

			auto ret = createPipeline();
			_data.insert({provider.key, std::make_any<std::weak_ptr<T>>(ret)});
			return ret;
		}

	private:
		std::mutex _dataMtx{};
		std::unordered_map<std::string, std::any> _data{};
	};
}// namespace squi::Store