#pragma once
#include "cmmn.h"
#include "device.h"

namespace vkng {
	struct swap_chain {
		device* dev;
		vk::UniqueSwapchainKHR sch;
		vector<vk::Image> images;
		vector<vk::UniqueImageView> image_views;
		vk::Extent2D extent;
		vk::Format format;
		vk::UniqueSemaphore image_ava_sp, render_fin_sp;

		unique_ptr<image> depth_buf;
		vk::UniqueImageView depth_view;

		result<uint32_t, vk::Result> aquire_next();
		void present(uint32_t index);
		void recreate(app* app);

		vector<vk::UniqueFramebuffer> create_framebuffers(vk::RenderPass rnp, function<void(size_t, vector<vk::ImageView>&)> additional_image_views = [](auto, auto) {});

		swap_chain(app* app, device* dev);
		~swap_chain();
	private:
		void create(app* app);
	};
}
