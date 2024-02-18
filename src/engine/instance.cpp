#include "instance.hpp"
#include "ranges"
#include "set"
#include <print>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>


using namespace Engine;

#if NDEBUG
constexpr bool debugBuild = false;
#else
constexpr bool debugBuild = true;
#endif

static const std::vector<const char *> validationLayers{
	"VK_LAYER_KHRONOS_validation",
};

const std::vector<const char *> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

Instance::Instance() : window(800, 600, "Vulkan window"),
					   instance(createInstance()),
					   surface(createSurface()),
					   physicalDevice(selectPhysicalDevice()),
					   device(createLogicalDevice()),
					   graphicsQueue(createGraphicsQueue()),
					   presentQueue(createPresentQueue()),
					   swapChain(createSwapChain(false)),
					   swapChainImageFormat(createSwapChainImageFormat()),
					   swapChainExtent(createExtent()),
					   swapChainImages(createSwapChainImages()),
					   swapChainImageViews(createImageViews()),
					   renderPass(createRenderPass()),
					   swapChainFramebuffers(createFramebuffers()),
					   frames{[&] {
						   std::vector<Frame> ret{};
						   ret.reserve(FrameBuffer);
						   auto props = findQueueFamilies(physicalDevice);
						   for (size_t i = 0; i < FrameBuffer; i++) {
							   ret.emplace_back(i, device, props.graphicsFamily.value());
						   }

						   return ret;
					   }()},
					   currentFrame(frames.front()) {}

void Engine::Instance::recreateSwapChain() {
	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(window.ptr, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window.ptr, &width, &height);
		glfwWaitEvents();
	}
	device.waitIdle();

	swapChain = createSwapChain(true);
	swapChainImageFormat = createSwapChainImageFormat();
	swapChainExtent = createExtent();
	swapChainImages = createSwapChainImages();
	swapChainImageViews = createImageViews();
	renderPass = createRenderPass();
	swapChainFramebuffers = createFramebuffers();

	for (auto &frame: frames) {
		frame.recreateCommandBuffer(device);
	}
}

vk::raii::Instance Engine::Instance::createInstance() const {
	if (debugBuild && !checkValidationLayers()) {
		throw std::runtime_error("The required validation layers are not available");
	}

	vk::ApplicationInfo appInfo{
		.pApplicationName = "App name",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "EngineVK",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_3,
	};

	uint32_t glfwExtCount{};
	const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtCount);

	vk::InstanceCreateInfo createInfo{
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = 0,
		.enabledExtensionCount = glfwExtCount,
		.ppEnabledExtensionNames = glfwExtensions,
	};

	if constexpr (debugBuild) {
		createInfo.ppEnabledLayerNames = validationLayers.data();
		createInfo.enabledLayerCount = validationLayers.size();
	}

	return {context, createInfo};
}

vk::raii::SurfaceKHR Engine::Instance::createSurface() const {
	VkSurfaceKHR surface_ = nullptr;
	if (glfwCreateWindowSurface(*instance, window.ptr, nullptr, &surface_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
	return {instance, surface_};
}

vk::raii::PhysicalDevice Engine::Instance::selectPhysicalDevice() const {
	auto devices = instance.enumeratePhysicalDevices();

	if (devices.empty()) {
		throw std::runtime_error("No GPUs with Vulkan support found");
	}

	// bool found = false;
	for (const auto &device: devices) {
		auto extensions = device.enumerateDeviceExtensionProperties();
		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto &extension: extensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		if (!requiredExtensions.empty()) continue;

		auto formats = device.getSurfaceFormatsKHR(*surface);
		auto presentModes = device.getSurfacePresentModesKHR(*surface);

		if (formats.empty() || presentModes.empty()) continue;

		if (!findQueueFamilies(device).isComplete()) continue;

		return device;
	}

	throw std::runtime_error("Failed to find a suitable GPU");
}

vk::raii::Device Engine::Instance::createLogicalDevice() const {
	auto indices = findQueueFamilies(physicalDevice);

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
		.enabledLayerCount = 0,
		.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data(),
		.pEnabledFeatures = &deviceFeatures,
	};

	if constexpr (debugBuild) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	return {physicalDevice, createInfo};
}

vk::raii::Queue Engine::Instance::createGraphicsQueue() const {
	auto indices = findQueueFamilies(physicalDevice);
	return device.getQueue(indices.graphicsFamily.value(), 0);
}

vk::raii::Queue Engine::Instance::createPresentQueue() const {
	auto indices = findQueueFamilies(physicalDevice);
	return device.getQueue(indices.presentFamily.value(), 0);
}

vk::raii::SwapchainKHR Engine::Instance::createSwapChain(bool recreating) {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	vk::PresentModeKHR presentMode = chooseSwapPresentMode();
	vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR createInfo{
		.surface = *surface,
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		.preTransform = swapChainSupport.capabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = presentMode,
		.clipped = true,
		.oldSwapchain = nullptr,
	};

	if (recreating) createInfo.setOldSwapchain(*swapChain);

	auto indices = findQueueFamilies(physicalDevice);
	std::array<uint32_t, 2> queueFamilyIndices{indices.graphicsFamily.value(), indices.presentFamily.value()};

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	} else {
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		createInfo.queueFamilyIndexCount = 0;    // Optional
		createInfo.pQueueFamilyIndices = nullptr;// Optional
	}

	return device.createSwapchainKHR(createInfo);
}

std::vector<vk::Image> Engine::Instance::createSwapChainImages() const {
	return (*device).getSwapchainImagesKHR(*swapChain);
}

vk::Format Engine::Instance::createSwapChainImageFormat() {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
	vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	return surfaceFormat.format;
}

vk::Extent2D Engine::Instance::createExtent() {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
	vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
	return extent;
}

std::vector<vk::raii::ImageView> Engine::Instance::createImageViews() {
	std::vector<vk::raii::ImageView> ret{};
	ret.reserve(swapChainImages.size());

	for (auto index: std::views::iota(0ull, swapChainImages.size())) {
		vk::ImageViewCreateInfo createInfo{
			.image = swapChainImages.at(index),
			.viewType = vk::ImageViewType::e2D,
			.format = swapChainImageFormat,
			.components = {
				.r = vk::ComponentSwizzle::eIdentity,
				.g = vk::ComponentSwizzle::eIdentity,
				.b = vk::ComponentSwizzle::eIdentity,
				.a = vk::ComponentSwizzle::eIdentity,
			},
			.subresourceRange = {
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};

		ret.emplace_back(device.createImageView(createInfo));
	}

	return ret;
}

vk::raii::RenderPass Engine::Instance::createRenderPass() {
	vk::AttachmentDescription colorAttachment{
		.format = swapChainImageFormat,
		.samples = vk::SampleCountFlagBits::e1,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
		.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
		.initialLayout = vk::ImageLayout::eUndefined,
		.finalLayout = vk::ImageLayout::ePresentSrcKHR,
	};

	vk::AttachmentReference colorAttachmentRef{
		.attachment = 0,
		.layout = vk::ImageLayout::eColorAttachmentOptimal,
	};

	vk::SubpassDescription subpass{
		.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
	};

	vk::SubpassDependency dependency{
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
		// .srcAccessMask = 0,
		.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
		.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
	};

	vk::RenderPassCreateInfo renderPassInfo{
		.attachmentCount = 1,
		.pAttachments = &colorAttachment,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dependency,
	};

	return device.createRenderPass(renderPassInfo);
}

std::vector<vk::raii::Framebuffer> Engine::Instance::createFramebuffers() {
	std::vector<vk::raii::Framebuffer> ret{};
	ret.reserve(swapChainImageViews.size());

	for (auto index: std::views::iota(0ull, swapChainImageViews.size())) {
		vk::FramebufferCreateInfo framebufferInfo{
			.renderPass = *renderPass,
			.attachmentCount = 1,
			.pAttachments = &*swapChainImageViews.at(index),
			.width = swapChainExtent.width,
			.height = swapChainExtent.height,
			.layers = 1,
		};

		ret.emplace_back(device.createFramebuffer(framebufferInfo));
	}

	return ret;
}

bool Engine::Instance::checkValidationLayers() {
	auto availableLayers = vk::enumerateInstanceLayerProperties();

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

Engine::Instance::QueueFamilyIndices Engine::Instance::findQueueFamilies(const vk::raii::PhysicalDevice &device) const {
	QueueFamilyIndices indices{};

	auto queueFamilies = device.getQueueFamilyProperties();

	for (const auto &[index, queueFamily]: queueFamilies | std::views::enumerate) {

		if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
			indices.graphicsFamily = index;
		}

		if (device.getSurfaceSupportKHR(index, *surface)) {
			indices.presentFamily = index;
		}

		if (indices.isComplete()) break;
	}

	return indices;
}

Engine::Instance::SwapChainSupportDetails Engine::Instance::querySwapChainSupport(const vk::raii::PhysicalDevice &device) const {
	SwapChainSupportDetails details;

	details.capabilities = device.getSurfaceCapabilitiesKHR(*surface);
	details.formats = device.getSurfaceFormatsKHR(*surface);
	details.presentModes = device.getSurfacePresentModesKHR(*surface);

	return details;
}

vk::Extent2D Engine::Instance::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) const {
	if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
		return capabilities.currentExtent;

	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(window.ptr, &width, &height);

	vk::Extent2D actualExtent = {
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height),
	};

	actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return actualExtent;
}

vk::PresentModeKHR Engine::Instance::chooseSwapPresentMode() {
	return vk::PresentModeKHR::eImmediate;
}

vk::SurfaceFormatKHR Engine::Instance::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
	for (const auto &availableFormat: availableFormats) {
		if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return availableFormat;
		}
	}

	return availableFormats.front();
}