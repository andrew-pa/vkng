#pragma once
#include "cmmn.h"
#include "app.h"
#include "vk_mem_alloc.h"

namespace vkng {
	struct buffer {
		struct device* dev;
		VkBuffer buf;
		VmaAllocation alloc;
		buffer() {}
		buffer(device* dev, vk::DeviceSize size, vk::BufferUsageFlags bufuse, vk::MemoryPropertyFlags memuse, 
			optional<void**> persistant_map = {});

		operator vk::Buffer() {
			return vk::Buffer(buf);
		}

		void* map();
		void unmap();

		~buffer();
	};
	struct image {
		struct device* dev;
		VkImage img;
		VmaAllocation alloc;
		image(device* dev, vk::ImageCreateFlags createflags, vk::ImageType type, vk::Extent3D size, vk::Format fmt,
			vk::ImageTiling til, vk::ImageUsageFlags use, vk::MemoryPropertyFlags memuse, size_t mip_count, size_t array_layers,
			optional<vk::UniqueImageView*> iv = {}, vk::ImageViewType iv_type = {}, vk::ImageSubresourceRange iv_sr = {});
		image(device* dev, vk::ImageType type, vk::Extent3D size, vk::Format fmt,
			vk::ImageTiling til, vk::ImageUsageFlags use, vk::MemoryPropertyFlags memuse,
			optional<vk::UniqueImageView*> iv = {}, vk::ImageViewType iv_type = {}, vk::ImageSubresourceRange iv_sr = {})
			: image(dev, vk::ImageCreateFlags(), type, size, fmt, til, use, memuse, 1, 1, iv, iv_type, iv_sr) {}


		// Automatically generate mipmaps using the GPU
		void generate_mipmaps(size_t w, size_t h, vk::CommandBuffer cb, size_t layer_count = 1,
			vk::ImageLayout final_layout = vk::ImageLayout::eShaderReadOnlyOptimal);

		static inline size_t calculate_mipmap_count(size_t w, size_t h) {
			return floor(log2(glm::min((float)w, (float)h)));
		}

		operator vk::Image() {
			return vk::Image(img);
		}

		~image();
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
