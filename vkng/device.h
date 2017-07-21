#pragma once
#include "cmmn.h"
#include "app.h"

namespace vkng {
	struct device {
		vk::PhysicalDevice  pdevice;
		struct queue_families {
			int graphics = -1;
			int present = -1;

			queue_families() {}
			queue_families(vk::PhysicalDevice pd, app* app);

			bool complete() {
				return graphics >= 0 && present >= 0;
			}
		} qu_fam;
		vk::UniqueDevice dev;
		vk::Queue graphics_qu;
		vk::Queue present_qu;

		device(app* app);

		~device();
	};
}
