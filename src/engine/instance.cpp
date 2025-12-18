#include "instance.hpp"
#include "ranges"
#include "vulkanIncludes.hpp"

#include "vulkan.hpp"

#include "GLFW/glfw3.h"
#include <GLFW/glfw3native.h>

#include <utility>

using namespace Engine;

Instance::Instance(WindowOptions options)
	: window(std::move(options)),
	  surface(createSurface()),
	  swapChainExtent(createExtent()),
	  swapChain(createSwapChain(false)),
	  swapChainImageFormat(createSwapChainImageFormat()),
	  swapChainImages(createSwapChainImages()),
	  swapChainImageViews(createImageViews()),
	  renderPass(createRenderPass()),
	  swapChainFramebuffers(createFramebuffers()),
	  frames{[&] {
		  std::vector<Frame> ret{};
		  ret.reserve(swapChainImages.size());
		  auto props = Engine::Vulkan::findQueueFamilies(Vulkan::physicalDevice());
		  for (size_t i = 0; i < swapChainImages.size(); i++) {
			  ret.emplace_back(i, Vulkan::device(), props.graphicsFamily.value());
		  }

		  return ret;
	  }()},
	  currentFrame(frames.front()) {}

void Engine::Instance::recreateSwapChain() {
	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(window.ptr, &width, &height);
	if (width == 0 || height == 0) return;
	Vulkan::device().waitIdle();

	auto swapChainSupport = querySwapChainSupport(Vulkan::physicalDevice());
	swapChainExtent = chooseSwapExtent(swapChainSupport.capabilities);
	swapChain = createSwapChain(true, swapChainSupport);
	swapChainImageFormat = createSwapChainImageFormat();
	swapChainImages = createSwapChainImages();
	swapChainImageViews = createImageViews();
	renderPass = createRenderPass();
	swapChainFramebuffers = createFramebuffers();

	for (auto &frame: frames) {
		frame.recreateCommandBuffer(Vulkan::device());
	}
}

vk::raii::SurfaceKHR Engine::Instance::createSurface() const {
#ifdef _WIN32
	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo{
		.hinstance = Vulkan::loader().has_value() ? Vulkan::loader()->m_library : Vulkan::fallbackLoader()->m_library,
		.hwnd = glfwGetWin32Window(window.ptr),
	};
	return Vulkan::instance().createWin32SurfaceKHR(surfaceCreateInfo);
#else
	// #error "Platform not supported"
	// vk::WaylandSurfaceCreateInfoKHR surfaceCreateInfo{
	// 	.display = glfwGetWaylandDisplay(),
	// 	.surface = glfwGetWaylandWindow(window.ptr),
	// };
	// return instance.createWaylandSurfaceKHR(surfaceCreateInfo);

	VkSurfaceKHR _surface = nullptr;
	glfwCreateWindowSurface(*Vulkan::instance(), window.ptr, nullptr, &_surface);
	return {Vulkan::instance(), _surface};
#endif
}

vk::raii::SwapchainKHR Engine::Instance::createSwapChain(bool recreating) {
	return createSwapChain(recreating, querySwapChainSupport(Vulkan::physicalDevice()));
}

vk::raii::SwapchainKHR Engine::Instance::createSwapChain(bool recreating, const SwapChainSupportDetails& swapChainSupport) {
	vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	vk::PresentModeKHR presentMode = chooseSwapPresentMode();

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR createInfo{
		.surface = *surface,
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = swapChainExtent,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		.preTransform = swapChainSupport.capabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = presentMode,
		.clipped = true,
		.oldSwapchain = nullptr,
	};

	if (recreating) createInfo.setOldSwapchain(*swapChain);

	auto indices = Vulkan::findQueueFamilies(Vulkan::physicalDevice());
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

	return Vulkan::device().createSwapchainKHR(createInfo);
}

std::vector<vk::Image> Engine::Instance::createSwapChainImages() const {
	return swapChain.getImages();
}

vk::Format Engine::Instance::createSwapChainImageFormat() {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(Vulkan::physicalDevice());
	vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	return surfaceFormat.format;
}

vk::Extent2D Engine::Instance::createExtent() {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(Vulkan::physicalDevice());
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

		ret.emplace_back(Vulkan::device().createImageView(createInfo));
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

	return Vulkan::device().createRenderPass(renderPassInfo);
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

		ret.emplace_back(Vulkan::device().createFramebuffer(framebufferInfo));
	}

	return ret;
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