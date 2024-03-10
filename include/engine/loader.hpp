#pragma once

#include "vulkanIncludes.hpp"
#if defined(__unix__) || defined(__APPLE__) || defined(__QNX__) || defined(__Fuchsia__)
#include <dlfcn.h>
#elif defined(_WIN32)
typedef struct HINSTANCE__ *HINSTANCE;
#if defined(_WIN64)
typedef int64_t(__stdcall *FARPROC)();
#else
typedef int(__stdcall *FARPROC)();
#endif
extern "C" __declspec(dllimport) HINSTANCE __stdcall LoadLibraryA(char const *lpLibFileName);
extern "C" __declspec(dllimport) int __stdcall FreeLibrary(HINSTANCE hLibModule);
extern "C" __declspec(dllimport) FARPROC __stdcall GetProcAddress(HINSTANCE hModule, const char *lpProcName);
#endif

// Copied from vulkan.hpp since the whole class is disabled when VULKAN_HPP_ENABLE_DYNAMIC_LOADER_TOOL is set to 0
namespace Engine {
	class DynamicLoader {
	public:
		DynamicLoader(std::string const &vulkanLibraryName = {}) {
			if (!vulkanLibraryName.empty()) {
#if defined(__unix__) || defined(__APPLE__) || defined(__QNX__) || defined(__Fuchsia__)
				m_library = dlopen(vulkanLibraryName.c_str(), RTLD_NOW | RTLD_LOCAL);
#elif defined(_WIN32)
				m_library = ::LoadLibraryA(vulkanLibraryName.c_str());
#else
#error unsupported platform
#endif
			} else {
#if defined(__unix__) || defined(__QNX__) || defined(__Fuchsia__)
				m_library = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
				if (m_library == nullptr) {
					m_library = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
				}
#elif defined(__APPLE__)
				m_library = dlopen("libvulkan.dylib", RTLD_NOW | RTLD_LOCAL);
				if (m_library == nullptr) {
					m_library = dlopen("libvulkan.1.dylib", RTLD_NOW | RTLD_LOCAL);
				}
#elif defined(_WIN32)
				m_library = ::LoadLibraryA("vulkan-1.dll");
#else
#error unsupported platform
#endif
			}

#ifndef VULKAN_HPP_NO_EXCEPTIONS
			if (m_library == nullptr) {
				// NOTE there should be an InitializationFailedError, but msvc insists on the symbol does not exist within the scope of this function.
				throw std::runtime_error("Failed to load vulkan library!");
			}
#endif
		}

		DynamicLoader(DynamicLoader const &) = delete;

		DynamicLoader(DynamicLoader &&other) VULKAN_HPP_NOEXCEPT : m_library(other.m_library) {
			other.m_library = nullptr;
		}

		DynamicLoader &operator=(DynamicLoader const &) = delete;

		DynamicLoader &operator=(DynamicLoader &&other) VULKAN_HPP_NOEXCEPT {
			std::swap(m_library, other.m_library);
			return *this;
		}

		~DynamicLoader() VULKAN_HPP_NOEXCEPT {
			if (m_library) {
#if defined(__unix__) || defined(__APPLE__) || defined(__QNX__) || defined(__Fuchsia__)
				dlclose(m_library);
#elif defined(_WIN32)
				::FreeLibrary(m_library);
#else
#error unsupported platform
#endif
			}
		}

		template<typename T>
		T getProcAddress(const char *function) const VULKAN_HPP_NOEXCEPT {
#if defined(__unix__) || defined(__APPLE__) || defined(__QNX__) || defined(__Fuchsia__)
			return (T) dlsym(m_library, function);
#elif defined(_WIN32)
			return (T)::GetProcAddress(m_library, function);
#else
#error unsupported platform
#endif
		}

		[[nodiscard]] bool success() const VULKAN_HPP_NOEXCEPT {
			return m_library != nullptr;
		}

#if defined(__unix__) || defined(__APPLE__) || defined(__QNX__) || defined(__Fuchsia__)
		void *m_library;
#elif defined(_WIN32)
		::HINSTANCE m_library;
#else
#error unsupported platform
#endif
	};
}// namespace Engine