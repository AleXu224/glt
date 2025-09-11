#pragma once

#include <memory>
#include <string>

namespace squi::core {
	struct KeyBase {
	public:
		KeyBase() = default;
		KeyBase(const KeyBase &) = default;
		KeyBase(KeyBase &&) = delete;
		KeyBase &operator=(const KeyBase &) = default;
		KeyBase &operator=(KeyBase &&) = delete;
		virtual ~KeyBase() = default;
		// Compare with nullptr
		virtual bool operator==(std::nullptr_t) const {
			return false;
		}
		virtual bool operator==(const KeyBase &other) const = 0;
		[[nodiscard]] virtual std::size_t hash() const = 0;
		[[nodiscard]] virtual std::string toString() const = 0;

		operator std::shared_ptr<const KeyBase>(this auto &&self) {
			return std::make_shared<const std::remove_cvref_t<decltype(self)>>(std::forward<decltype(self)>(self));
		}
	};

	struct NullKey : public KeyBase {
		bool operator==(std::nullptr_t) const override {
			return true;
		}

		bool operator==(const KeyBase &other) const override {
			return dynamic_cast<const NullKey *>(&other) != nullptr;
		}

		std::size_t hash() const override {
			return 0;// All NullKeys are considered equal
		}

		std::string toString() const override {
			return "[NullKey]";
		}
	};

	const inline std::shared_ptr<const KeyBase> nullKey = std::make_shared<NullKey>();

	struct ValueKey : public KeyBase {
	private:
		std::string value;

	public:
		ValueKey(const std::string &value) : value(value) {}

		bool operator==(const KeyBase &other) const override {
			if (const auto *otherValueKey = dynamic_cast<const ValueKey *>(&other)) {
				return value == otherValueKey->value;
			}
			return false;
		}

		std::size_t hash() const override {
			return std::hash<std::string>{}(value);
		}

		std::string toString() const override {
			return "[ValueKey " + value + "]";
		}

		const std::string &getValue() const { return value; }
	};

	struct ObjectKey : public KeyBase {
	private:
		const void *object;

	public:
		ObjectKey(const void *object) : object(object) {}

		bool operator==(const KeyBase &other) const override {
			if (const auto *otherObjectKey = dynamic_cast<const ObjectKey *>(&other)) {
				return object == otherObjectKey->object;
			}
			return false;
		}

		std::size_t hash() const override {
			return std::hash<const void *>{}(object);
		}

		std::string toString() const override {
			return "[ObjectKey " + std::to_string(reinterpret_cast<uintptr_t>(object)) + "]";
		}

		const void *getObject() const { return object; }
	};

	using Key = std::shared_ptr<const KeyBase>;
}// namespace squi::core
