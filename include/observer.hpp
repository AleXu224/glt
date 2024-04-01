#pragma once
#include <functional>
#include <memory>

namespace squi {
	template<typename T>
	struct Observable {
		struct Observer;
		struct ControlBlock {
			std::vector<Observer> observers{};
		};
		using BlockPtr = std::shared_ptr<ControlBlock>;
		using UpdateFunc = std::function<void(const T &)>;
		BlockPtr _controlBlock = std::make_shared<ControlBlock>();

		static void _notify(const BlockPtr &controlBlock, const T &t) {
			for (auto &observer: controlBlock->observers) {
				observer.update(t);
			}
		}

		struct Observer {
			std::shared_ptr<ControlBlock> _controlBlock;
			UpdateFunc update;

			void notifyOthers(const T &t) const {
				_notify(_controlBlock, t);
			}
		};

		[[nodiscard]] static Observer &_observe(const BlockPtr &controlBlock, const UpdateFunc &updateFunc) {
			return controlBlock->observers.emplace_back(controlBlock, updateFunc);
		}

		void notify(const T &t) const {
			_notify(_controlBlock, t);
		}

		[[nodiscard]] Observer &observe(const UpdateFunc &updateFunc) const {
			return _observe(_controlBlock, updateFunc);
		}

		Observable() = default;
	};

	template<class T>
	using Observer = Observable<T>::Observer;

	struct VoidObservable {
		struct Observer;
		struct ControlBlock {
			std::vector<Observer> observers{};
		};
		using BlockPtr = std::shared_ptr<ControlBlock>;
		using UpdateFunc = std::function<void()>;
		BlockPtr _controlBlock = std::make_shared<ControlBlock>();

		static void _notify(const BlockPtr &controlBlock) {
			for (auto &observer: controlBlock->observers) {
				observer.update();
			}
		}

		struct Observer {
			std::shared_ptr<ControlBlock> _controlBlock;
			UpdateFunc update;

			void notifyOthers() const {
				_notify(_controlBlock);
			}
		};

		[[nodiscard]] static Observer &_observe(const BlockPtr &controlBlock, const UpdateFunc &updateFunc) {
			return controlBlock->observers.emplace_back(controlBlock, updateFunc);
		}

		void notify() const {
			_notify(_controlBlock);
		}

		[[nodiscard]] Observer &observe(const UpdateFunc &updateFunc) const {
			return _observe(_controlBlock, updateFunc);
		}

		VoidObservable() = default;
	};

	using VoidObserver = VoidObservable::Observer;
}// namespace squi