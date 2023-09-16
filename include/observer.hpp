#pragma once
#include <functional>
#include <memory>
#include <utility>

#include "functional"

namespace squi {
	template<typename T>
	struct Observable : std::enable_shared_from_this<Observable<T>> {
		struct Observer {
			std::function<void(const T &)> update;
		};

		// FIXME: old observers are never removed
		std::vector<std::weak_ptr<Observer>> observers{};

		void notify(const T &t) {
			for (auto &observer: observers) {
				if (auto o = observer.lock())
					o->update(t);
			}
		}

		[[nodiscard]] std::shared_ptr<Observer> observe(std::function<void(const T &)> update) {
			std::shared_ptr<Observer> observer = std::make_shared<Observer>(std::move(update));
			observers.emplace_back(observer);
			return observer;
		}

		[[nodiscard]] static std::shared_ptr<Observable<T>> create() {
			return std::make_shared<Observable<T>>(Observable<T>{});
		}

	private:
		Observable() = default;
	};

	struct VoidObservable : std::enable_shared_from_this<VoidObservable> {
		struct Observer {
			std::weak_ptr<VoidObservable> observable;
			std::function<void()> update;

			void notifyObserver() const {
				if (auto o = observable.lock())
					o->notify();
			}
		};

		std::vector<std::weak_ptr<Observer>> observers{};

		void notify() {
			for (auto &observer: observers) {
				if (auto o = observer.lock())
					o->update();
			}
		}

		[[nodiscard]] std::shared_ptr<Observer> observe(std::function<void()> update) {
			std::shared_ptr<Observer> observer = std::make_shared<Observer>(weak_from_this(), std::move(update));
			observers.emplace_back(observer);
			return observer;
		}

		[[nodiscard]] static std::shared_ptr<VoidObservable> create() {
			return std::make_shared<VoidObservable>(VoidObservable{});
		}

	private:
		VoidObservable() = default;
	};
}// namespace squi