#pragma once
#include "array"
#include "vulkanIncludes.hpp"
#include <cstring>
#include <span>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>


namespace Engine {
	struct Shader {
		size_t bufferIndex = 0;

		vk::raii::ShaderModule module = nullptr;

		template<size_t Size>
		Shader(vk::raii::Device &device, const std::array<char, Size> &data) {
			vk::ShaderModuleCreateInfo createInfo{
				.codeSize = data.size() * sizeof(std::byte),
				.pCode = reinterpret_cast<uint32_t *>(const_cast<char *>(data.data())),
			};

			module = device.createShaderModule(createInfo);
		}

		Shader(vk::raii::Device &device, const std::span<const char> data) {
			vk::ShaderModuleCreateInfo createinfo{
				.codeSize = data.size() * sizeof(std::byte),
				.pCode = reinterpret_cast<uint32_t *>(const_cast<char *>(data.data())),
			};

			module = device.createShaderModule(createinfo);
		}
	};
}// namespace Engine