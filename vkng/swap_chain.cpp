#include "swap_chain.h"

namespace vkng {
	swap_chain::swap_chain(app* app, device* dev) {
		auto surf_caps = dev->pdevice.getSurfaceCapabilitiesKHR(app->surface);
		uint32_t image_count = surf_caps.minImageCount + 1;
		if (surf_caps.maxImageCount > 0 && image_count > surf_caps.maxImageCount)
			image_count = surf_caps.maxImageCount;
		vk::SwapchainCreateInfoKHR cfo;
		cfo.surface = app->surface;
		cfo.minImageCount = image_count;
		format = cfo.imageFormat = vk::Format::eB8G8R8A8Unorm; //basically assume device is not garbage
		cfo.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
		auto winsize = app->size();
		extent = cfo.imageExtent = vk::Extent2D(winsize.x, winsize.y);
		cfo.imageArrayLayers = 1;
		cfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
		if (dev->qu_fam.graphics != dev->qu_fam.present) {
			cfo.imageSharingMode = vk::SharingMode::eConcurrent;
			cfo.queueFamilyIndexCount = 2;
			uint32_t qfi[] = { (uint32_t)dev->qu_fam.graphics, (uint32_t)dev->qu_fam.present };
			cfo.pQueueFamilyIndices = qfi;
		}
		else {
			cfo.imageSharingMode = vk::SharingMode::eExclusive;
		}
		cfo.preTransform = surf_caps.currentTransform;
		cfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		cfo.presentMode = vk::PresentModeKHR::eFifo;
		cfo.clipped = true;
		
		sch = dev->dev->createSwapchainKHRUnique(cfo);

		images = dev->dev->getSwapchainImagesKHR(sch.get());
		vk::ImageViewCreateInfo ivcfo;
		ivcfo.viewType = vk::ImageViewType::e2D;
		ivcfo.format = format;
		ivcfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		ivcfo.subresourceRange.baseMipLevel = 0;
		ivcfo.subresourceRange.levelCount = 1;
		ivcfo.subresourceRange.baseArrayLayer = 0;
		ivcfo.subresourceRange.layerCount = 1;
		for (auto img : images) {
			ivcfo.image = img;
			image_views.push_back(dev->dev->createImageViewUnique(ivcfo));
		}
	}

	swap_chain::~swap_chain() {}
}