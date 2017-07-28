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

		vk::UniqueSemaphore image_ava_sp, render_fin_sp;

		result<uint32_t, vk::Result> aquire_next(device* dev);
		void present(device* dev, uint32_t index);
		void recreate(app* app, device* dev);

		swap_chain(app* app, device* dev);
		~swap_chain();
	private:
		void create(app* app, device* dev);
	};
}
