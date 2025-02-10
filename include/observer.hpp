#pragma once
#include "stateContainer.hpp"
#include "vector"
#include <functional>
#include <memory>
#include <string_view>


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
			Observer ret{._controlBlock = controlBlock, ._update = std::make_shared<UpdateFunc>(updateFunc)};
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

	// An observer meant the synchronize multiple widgets, by notifying when the required count has been met, regardless of the initialization order
	// Used in circumstances like waiting for certain widgets to be initialized before performing an action.
	struct CountObserver {
		struct ControlBlock {
			size_t target = 0;
			size_t current = 0;
			VoidObserver counter{};
			VoidObservable readyEvent{};
		};

		CountObserver(size_t target)
			: _controlBlock(std::make_shared<ControlBlock>(ControlBlock{
				  .target = target,
			  })) {
			_controlBlock->counter = VoidObservable{}.observe([blockPtr = std::weak_ptr<ControlBlock>{_controlBlock}]() {
				if (auto controlBlock = blockPtr.lock()) {
					controlBlock->current++;
					if (controlBlock->current == controlBlock->target) {
						controlBlock->readyEvent.notify();
					}
				}
			});
		}

		void notify() const {
			_controlBlock->counter.notifyOthers();
		}

		[[nodiscard]] VoidObserver observe(const std::function<void()> &updateFunc) const {
			if (_controlBlock->current >= _controlBlock->target && updateFunc) {
				updateFunc();
			}
			return _controlBlock->readyEvent.observe(updateFunc);
		}

	private:
		std::shared_ptr<ControlBlock> _controlBlock{};
	};

	template<class W, class O, class F>
	void observe(W &&widget, O &&observable, F &&func) {
		if constexpr (std::is_pointer_v<std::decay_t<W>>)
			widget->customState.add(observable.observe(func));
		else
			widget.customState.add(observable.observe(func));
	}
	template<class W, class O, class F>
	void observe(std::string_view name, W &&widget, O &&observable, F &&func) {
		if constexpr (std::is_pointer_v<std::decay_t<W>>)
			widget->customState.add(StateContainer(name, observable.observe(func)));
		else
			widget.customState.add(StateContainer(name, observable.observe(func)));
	}
}// namespace squi