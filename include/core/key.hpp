#pragma once

#include <memory>
#include <string>

namespace squi::core {
	struct Key {
	public:
		virtual ~Key() = default;
		virtual bool operator==(const Key &other) const = 0;
		virtual std::size_t hash() const = 0;
		virtual std::string toString() const = 0;
	};

	struct NullKey : public Key {
		bool operator==(const Key &other) const override {
			return dynamic_cast<const NullKey *>(&other) != nullptr;
		}

		std::size_t hash() const override {
			return 0;// All NullKeys are considered equal
		}

		std::string toString() const override {
			return "[NullKey]";
		}
	};

	constexpr NullKey nullKey{};

	struct ValueKey : public Key {
	private:
		std::string value;

	public:
		ValueKey(const std::string &value) : value(std::move(value)) {}

		bool operator==(const Key &other) const override {
			if (auto *otherValueKey = dynamic_cast<const ValueKey *>(&other)) {
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

	struct ObjectKey : public Key {
	private:
		const void *object;

	public:
		ObjectKey(const void *object) : object(object) {}

		bool operator==(const Key &other) const override {
			if (auto *otherObjectKey = dynamic_cast<const ObjectKey *>(&other)) {
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

	using KeyPtr = std::unique_ptr<Key>;
}// namespace squi::core
