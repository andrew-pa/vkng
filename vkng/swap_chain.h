#pragma once
#include "cmmn.h"
#include "device.h"

namespace vkng {
	struct swap_chain {
		vk::UniqueSwapchainKHR sch;
		vector<vk::Image> images;
		vector<vk::UniqueImageView> image_views;
		vk::Extent2D extent;
		vk::Format format;

		swap_chain(app* app, device* dev);
		~swap_chain();
	};
}
