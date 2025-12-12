#include "vulkan.hpp"

#include "print"
#include <GLFW/glfw3.h>
#include <set>

#if NDEBUG
constexpr bool debugBuild = false;
#else
constexpr bool debugBuild = true;
#endif

static const std::vector<const char *> validationLayers{
	"VK_LAYER_KHRONOS_validation",
};

inline std::vector<const char *> &deviceExtensions() {
	static std::vector<const char *> _{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
	return _;
};

Engine::Vulkan::QueueFamilyIndices Engine::Vulkan::findQueueFamilies(const vk::raii::PhysicalDevice &physicalDevice) {
	Engine::Vulkan::QueueFamilyIndices indices{};

	auto queueFamilies = physicalDevice.getQueueFamilyProperties();

	for (size_t index = 0; queueFamilies.size(); index++) {
		auto &queueFamily = queueFamilies.at(index);

		if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
			indices.graphicsFamily = index;
#ifndef _WIN32
			// Checking if presentation is supported on linux is more complicated so i'll just take a gamble
			indices.presentFamily = index;
#endif
		}

#ifdef _WIN32
		if (physicalDevice.getWin32PresentationSupportKHR(index)) {
			indices.presentFamily = index;
		}
#endif

		if (indices.isComplete()) break;
	}

	return indices;
}

Engine::LockedResource<vk::raii::Queue> Engine::Vulkan::getGraphicsQueue() {
	static std::mutex mtx{};
	auto indices = Vulkan::findQueueFamilies(Vulkan::physicalDevice());
	return {std::scoped_lock{mtx}, Vulkan::device().resource.getQueue(indices.graphicsFamily.value(), 0)};
}

Engine::LockedResource<vk::raii::Queue> Engine::Vulkan::getPresentQueue() {
	static std::mutex mtx{};
	auto indices = Vulkan::findQueueFamilies(Vulkan::physicalDevice());
	return {std::scoped_lock{mtx}, Vulkan::device().resource.getQueue(indices.presentFamily.value(), 0)};
}

std::pair<vk::raii::CommandPool, vk::raii::CommandBuffer> Engine::Vulkan::makeCommandBuffer() {
	auto props = Vulkan::findQueueFamilies(Vulkan::physicalDevice());

	vk::CommandPoolCreateInfo poolInfo{
		.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = props.graphicsFamily.value(),
	};

	auto commandPool = Vulkan::device().resource.createCommandPool(poolInfo);

	return {
		std::move(commandPool),
		std::move(Vulkan::device().resource.allocateCommandBuffers(vk::CommandBufferAllocateInfo{
																	   .commandPool = *commandPool,
																	   .level = vk::CommandBufferLevel::ePrimary,
																	   .commandBufferCount = 1,
																   })
					  .front())
	};
}

void Engine::Vulkan::finishCommandBuffer(vk::raii::CommandBuffer &cmd) {
	vk::SubmitInfo submitInfo{
		.commandBufferCount = 1,
		.pCommandBuffers = &*cmd,
	};

	vk::raii::Fence fence{Vulkan::device().resource, vk::FenceCreateInfo{}};

	auto graphicsQueue = Vulkan::getGraphicsQueue();
	graphicsQueue.resource.submit(submitInfo, *fence);

	auto res = Vulkan::device().resource.waitForFences(*fence, true, 100000000);
	if (res != vk::Result::eSuccess) {
		throw std::runtime_error("Failed finishing the command buffer");
	}
}

std::optional<Engine::DynamicLoader> &Engine::Vulkan::loader() {
	static std::optional<DynamicLoader> _ = []() {
		try {
			return std::optional{DynamicLoader{}};
		} catch (std::exception &) {
			std::println("System doesn't have a vulkan loader");
			return std::optional<Engine::DynamicLoader>{};
		}
	}();

	return _;
}
std::optional<Engine::DynamicLoader> &Engine::Vulkan::fallbackLoader() {
	const std::string fallbackLoader{
#ifdef _WIN32
		"vk_swiftshader.dll"
#else
		// #error "Platform not supported"
		"vk_swiftshader.dll"
#endif
	};

	static std::optional<DynamicLoader> _ = [&]() {
		try {
			return std::optional{DynamicLoader{fallbackLoader}};
		} catch (std::exception &) {
			std::println("Loading fallback {} failed", fallbackLoader);
			return std::optional<Engine::DynamicLoader>{};
		}
	}();

	return _;
}

vk::raii::Context &Engine::Vulkan::context() {
	auto *loaderPtr = &loader();
	if (!loaderPtr->has_value()) loaderPtr = &fallbackLoader();
	if (!loaderPtr->has_value()) throw std::runtime_error("Failed finding a suitable loader");

	static vk::raii::Context _{loaderPtr->value().getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr")};
	// static vk::raii::Context _;
	return _;
}

bool Engine::Vulkan::checkValidationLayers() {
	auto availableLayers = context().enumerateInstanceLayerProperties();

	for (const auto &layer: validationLayers) {
		bool layerFound = false;
		for (const auto &layerProps: availableLayers) {
			if (strcmp(layer, layerProps.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) return false;
	}

	return true;
}

vk::raii::Instance &Engine::Vulkan::instance() {
	static vk::raii::Instance _ = []() {
		if (debugBuild && !checkValidationLayers()) {
			std::println("The required validation layers are not available, proceeding without them");
			validationLayersAvailable = false;
		}

		vk::ApplicationInfo appInfo{
			.pApplicationName = "App name",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "EngineVK",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_API_VERSION_1_3,
		};

		uint32_t glfwExtCount{};
		[[maybe_unused]] static bool initGlfw = []() {
			glfwInit();
			return true;
		}();
		const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtCount);

		vk::InstanceCreateInfo createInfo{
			.pApplicationInfo = &appInfo,
			.enabledLayerCount = 0,
			.enabledExtensionCount = glfwExtCount,
			.ppEnabledExtensionNames = glfwExtensions,
		};

		if (debugBuild && validationLayersAvailable) {
			createInfo.ppEnabledLayerNames = validationLayers.data();
			createInfo.enabledLayerCount = validationLayers.size();
		}

		// FIXME: add logic for dealing with bad contexts
		return vk::raii::Instance{context(), createInfo};
	}();
	return _;
}

vk::raii::PhysicalDevice &Engine::Vulkan::physicalDevice() {

	static vk::raii::PhysicalDevice _ = []() {
		auto &deviceExt = deviceExtensions();
		auto devices = instance().enumeratePhysicalDevices();

		if (devices.empty()) {
			throw std::runtime_error("No GPUs with Vulkan support found");
		}

		for (const auto &device: devices) {
			auto extensions = device.enumerateDeviceExtensionProperties();
			std::set<std::string> requiredExtensions(deviceExt.begin(), deviceExt.end());

			for (const auto &extension: extensions) {
				requiredExtensions.erase(extension.extensionName);
			}

			if (!requiredExtensions.empty()) continue;

			if (!findQueueFamilies(device).isComplete()) continue;

			return device;
		}

		throw std::runtime_error("Failed to find a suitable GPU");
	}();
	return _;
}

Engine::LockedResource<vk::raii::Device &> Engine::Vulkan::device() {
	static std::mutex mtx{};
	static vk::raii::Device _ = []() {
		auto &deviceExt = deviceExtensions();
		auto indices = findQueueFamilies(physicalDevice());

		constexpr float queuePriority = 1.f;

		vk::PhysicalDeviceFeatures deviceFeatures;

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};
		std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		queueCreateInfos.reserve(uniqueQueueFamilies.size());
		for (uint32_t queueFamily: uniqueQueueFamilies) {
			queueCreateInfos.emplace_back(vk::DeviceQueueCreateInfo{
				.queueFamilyIndex = queueFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
			});
		}

		vk::DeviceCreateInfo createInfo{
			.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
			.pQueueCreateInfos = queueCreateInfos.data(),
			.enabledExtensionCount = static_cast<uint32_t>(deviceExt.size()),
			.ppEnabledExtensionNames = deviceExt.data(),
			.pEnabledFeatures = &deviceFeatures,
		};

		return vk::raii::Device{physicalDevice(), createInfo};
	}();
	return {std::scoped_lock{mtx}, _};
}
