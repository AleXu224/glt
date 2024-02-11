#pragma once

#include "instance.hpp"
#include "ranges"
#include <array>
#include <functional>
#include <print>
#include <utility>


namespace Engine {
	[[maybe_unused]] static uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties, Instance &instance) {
		auto memProperties = instance.physicalDevice.getMemoryProperties();

		for (auto i: std::views::iota(0u, memProperties.memoryTypeCount)) {
			if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	template<class T, size_t N, size_t... I>
	static inline std::array<T, N> _generateArray(const std::function<T(size_t)> &generator, const std::index_sequence<I...> &) {
		return {((void) I, generator(I))...};
	}
	
	template<class T, size_t N>
	static inline std::array<T, N> generateArray(const std::function<T(size_t)> &generator) {
		return _generateArray<T, N>(generator, std::make_index_sequence<N>{});
	}


	struct MoveNCopyDetector {
		MoveNCopyDetector() = default;
		MoveNCopyDetector(const MoveNCopyDetector&) {
			std::println("Copied!");
		}
		MoveNCopyDetector(MoveNCopyDetector &) {
			std::println("Copied!");
		}
		MoveNCopyDetector(const MoveNCopyDetector &&) {
			std::println("Moved!");
		}
		MoveNCopyDetector(MoveNCopyDetector &&) {
			std::println("Moved!");
		}

		MoveNCopyDetector &operator=(const MoveNCopyDetector&) {
			std::println("Copied!");
			return *this;
		}
		MoveNCopyDetector &operator=(MoveNCopyDetector &) {
			std::println("Copied!");
			return *this;
		}
		MoveNCopyDetector &operator=(const MoveNCopyDetector &&) {
			std::println("Moved!");
			return *this;
		}
		MoveNCopyDetector &operator=(MoveNCopyDetector &&) {
			std::println("Moved!");
			return *this;
		}

		const MoveNCopyDetector &operator=(const MoveNCopyDetector &) const {
			std::println("Copied!");
			return *this;
		}
		const MoveNCopyDetector &operator=(MoveNCopyDetector &) const {
			std::println("Copied!");
			return *this;
		}
		const MoveNCopyDetector &operator=(const MoveNCopyDetector &&) const {
			std::println("Moved!");
			return *this;
		}
		const MoveNCopyDetector &operator=(MoveNCopyDetector &&) const {
			std::println("Moved!");
			return *this;
		}
	};
}// namespace Engine