#pragma once
#include <functional>
#include <memory>
#include <print>
namespace squi {
	template<typename T>
	struct Observable {
		using UpdateFunc = std::function<void(const T &)>;
		using SharedUpdateFunc = std::shared_ptr<UpdateFunc>;
		using WeakUpdateFunc = std::weak_ptr<UpdateFunc>;
		struct Observer;
		struct ControlBlock {
			std::vector<WeakUpdateFunc> updateFuncs{};
		};
		using BlockPtr = std::shared_ptr<ControlBlock>;
		BlockPtr _controlBlock = std::make_shared<ControlBlock>();

		static void _notify(const BlockPtr &controlBlock, const T &t) {
			for (const auto &updateFunc: controlBlock->updateFuncs) {
				if (updateFunc.expired()) continue;
				(*updateFunc.lock())(t);
			}
		}

		struct Observer {
			std::shared_ptr<ControlBlock> _controlBlock;
			SharedUpdateFunc update;

			void notifyOthers(const T &t) const {
				if (!_controlBlock) return;
				_notify(_controlBlock, t);
			}
		};

		[[nodiscard]] static Observer _observe(const BlockPtr &controlBlock, const UpdateFunc &updateFunc) {
			Observer ret{controlBlock, std::make_shared<UpdateFunc>(updateFunc)};
			controlBlock->updateFuncs.emplace_back(ret.update);
			return ret;
		}

		void notify(const T &t) const {
			_notify(_controlBlock, t);
		}

		[[nodiscard]] Observer observe(const UpdateFunc &updateFunc) const {
			return _observe(_controlBlock, updateFunc);
		}

		Observable() = default;
	};

	template<class T>
	using Observer = Observable<T>::Observer;

	struct VoidObservable {
		using UpdateFunc = std::function<void()>;
		using SharedUpdateFunc = std::shared_ptr<UpdateFunc>;
		using WeakUpdateFunc = std::weak_ptr<UpdateFunc>;
		struct Observer;
		struct ControlBlock {
			std::vector<WeakUpdateFunc> updateFuncs{};
		};

		using BlockPtr = std::shared_ptr<ControlBlock>;
		BlockPtr _controlBlock = std::make_shared<ControlBlock>();

		static void _notify(const BlockPtr &controlBlock) {
			for (auto &updateFunc: controlBlock->updateFuncs) {
				if (updateFunc.expired()) continue;
				(*updateFunc.lock())();
			}
		}

		struct Observer {
			std::shared_ptr<ControlBlock> _controlBlock;
			SharedUpdateFunc _update;

			void notifyOthers() const {
				if (!_controlBlock) return;
				_notify(_controlBlock);
			}
		};

		[[nodiscard]] static Observer _observe(const BlockPtr &controlBlock, const UpdateFunc &updateFunc) {
			Observer ret{controlBlock, std::make_shared<UpdateFunc>(updateFunc)};
			controlBlock->updateFuncs.emplace_back(ret._update);
			return ret;
		}

		void notify() const {
			_notify(_controlBlock);
		}

		[[nodiscard]] Observer observe(const UpdateFunc &updateFunc) const {
			return _observe(_controlBlock, updateFunc);
		}

		VoidObservable() = default;
	};

	using VoidObserver = VoidObservable::Observer;
}// namespace squi