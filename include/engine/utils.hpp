#pragma once

#include "vulkan.hpp"

#include "ranges"
#include <array>
#include <functional>
#include <print>
#include <utility>


namespace Engine {
	[[maybe_unused]] static uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
		auto memProperties = Vulkan::physicalDevice().getMemoryProperties();

		for (auto i: std::views::iota(0u, memProperties.memoryTypeCount)) {
			if (typeFilter & (1U << i) && (memProperties.memoryTypes.at(i).propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	template<class T, size_t N, size_t... I>
	static inline std::array<T, N> _generateArray(const std::function<T(size_t)> &generator, const std::index_sequence<I...> & /*unused*/) {
		return {((void) I, generator(I))...};
	}

	template<class T, size_t N>
	static inline std::array<T, N> generateArray(const std::function<T(size_t)> &generator) {
		return _generateArray<T, N>(generator, std::make_index_sequence<N>{});
	}
}// namespace Engine