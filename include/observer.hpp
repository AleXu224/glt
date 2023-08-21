#pragma once
#include <functional>
#include <memory>
#include <utility>

#include "functional"

namespace squi {
	template<typename T>
	struct Observable {
		struct Observer {
			std::function<void(const T &)> update;
		};

		std::vector<std::weak_ptr<Observer>> observers{};
		void notify(const T &t) {
			for (auto &observer: observers) {
				if (auto o = observer.lock())
					o->update(t);
			}
		}

		std::shared_ptr<Observer> observe(std::function<void(const T &)> update) {
			std::shared_ptr<Observer> observer = std::make_shared<Observer>(std::move(update));
			observers.emplace_back(observer);
			return observer;
		}
	};

	struct VoidObservable {
		struct Observer {
			VoidObservable &observable;
			std::function<void()> update;
		};

		std::vector<Observer> observers{};

		void notify() {
			for (auto &observer: observers) {
				observer.update();
			}
		}

		Observer observe(std::function<void()> update) {
			observers.emplace_back(Observer{*this, std::move(update)});
			return observers.back();
		}
	};
}// namespace squi