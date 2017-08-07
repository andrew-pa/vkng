#pragma once
#include "cmmn.h"
#include "app.h"
#include "vk_mem_alloc.h"

namespace vkng {
	struct buffer {
		struct device* dev;
		VkBuffer buf;
		VmaAllocation alloc;
		buffer(device* dev, vk::DeviceSize size, vk::BufferUsageFlags bufuse, vk::MemoryPropertyFlags memuse, 
			optional<void**> persistant_map = {});

		operator vk::Buffer() {
			return vk::Buffer(buf);
		}

		void* map();
		void unmap();

		~buffer();
	};
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
		vk::UniqueCommandPool cmdpool;
		vk::Queue graphics_qu;
		vk::Queue present_qu;
		VmaAllocator allocator;

		device(app* app);

		vector<vk::UniqueCommandBuffer> alloc_cmd_buffers(size_t num = 1, vk::CommandBufferLevel lvl = vk::CommandBufferLevel::ePrimary);

		vk::UniqueDescriptorSetLayout create_desc_set_layout(vector<vk::DescriptorSetLayoutBinding> bindings);

		~device();
	};
}
