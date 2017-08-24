#include "device.h"
#include <set>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace vkng {

	device::device(app* app) {
		auto devices = app->instance.enumeratePhysicalDevices(); // just choose the first physical device for now
		pdevice = devices[0];
		auto pdevice_props = pdevice.getProperties();
		cout << "Physical Device: " << pdevice_props.deviceName << endl;
		qu_fam = queue_families(pdevice, app);
		assert(qu_fam.complete());
		
		vk::DeviceCreateInfo dcfo;
		vector<vk::DeviceQueueCreateInfo> qu_cfo;
		auto unique_qufam = set<int> { qu_fam.graphics, qu_fam.present };
		float fp = 1.f;
		for (int qf : unique_qufam) {
			qu_cfo.push_back(vk::DeviceQueueCreateInfo{
				vk::DeviceQueueCreateFlags(),
				(uint32_t)qf, 1, &fp
			});
		}
		dcfo.queueCreateInfoCount = qu_cfo.size();
		dcfo.pQueueCreateInfos = qu_cfo.data();

		vk::PhysicalDeviceFeatures devfeat;
		devfeat.samplerAnisotropy = VK_TRUE;
		dcfo.pEnabledFeatures = &devfeat;
		vector<const char*> layer_names{
#ifdef DEBUG
			"VK_LAYER_LUNARG_standard_validation",
#endif
		};
		dcfo.enabledLayerCount = layer_names.size();
		dcfo.ppEnabledLayerNames = layer_names.data();
		vector<const char*> ext = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		dcfo.enabledExtensionCount = ext.size();
		dcfo.ppEnabledExtensionNames = ext.data();
		dev = pdevice.createDeviceUnique(dcfo);

		graphics_qu = dev->getQueue(qu_fam.graphics, 0);
		present_qu = dev->getQueue(qu_fam.present, 0);

		cmdpool = dev->createCommandPoolUnique(vk::CommandPoolCreateInfo{ vk::CommandPoolCreateFlagBits::eResetCommandBuffer, (uint32_t)qu_fam.graphics });

		VmaAllocatorCreateInfo cfo = {};
		cfo.physicalDevice = (VkPhysicalDevice)pdevice;
		cfo.device = (VkDevice)dev.get();
		vmaCreateAllocator(&cfo, &allocator);
	}

	vector<vk::UniqueCommandBuffer> device::alloc_cmd_buffers(size_t num, vk::CommandBufferLevel lvl) {
		vk::CommandBufferAllocateInfo afo;
		afo.level = lvl;
		afo.commandPool = cmdpool.get();
		afo.commandBufferCount = num;
		return dev->allocateCommandBuffersUnique(afo);
	}

	vk::UniqueDescriptorSetLayout device::create_desc_set_layout(vector<vk::DescriptorSetLayoutBinding> bindings) {
		return dev->createDescriptorSetLayoutUnique(vk::DescriptorSetLayoutCreateInfo{ vk::DescriptorSetLayoutCreateFlags(), bindings.size(), bindings.data() });
	}

	device::~device() {
		vmaDestroyAllocator(allocator);
	}


	device::queue_families::queue_families(vk::PhysicalDevice pd, app* app) {
		auto qufams = pd.getQueueFamilyProperties();
		for (uint32_t i = 0; i < qufams.size() && !complete(); ++i) {
			if (qufams[i].queueCount <= 0) continue;
			if (qufams[i].queueFlags & vk::QueueFlagBits::eGraphics) {
				graphics = i;
			}
			if (pd.getSurfaceSupportKHR(i, app->surface)) {
				present = i;
			}
		}
	}
	
	buffer::buffer(device* dev, vk::DeviceSize size, vk::BufferUsageFlags bufuse,
			vk::MemoryPropertyFlags memuse, optional<void**> persistent_map) : dev(dev)
	{
		VmaMemoryRequirements mreq = {};
		mreq.flags = persistent_map ? VMA_MEMORY_REQUIREMENT_PERSISTENT_MAP_BIT : 0;
		mreq.requiredFlags = (VkMemoryPropertyFlags)memuse;
		VmaAllocationInfo alli;
		auto res = vmaCreateBuffer(dev->allocator, (VkBufferCreateInfo*)&vk::BufferCreateInfo{ vk::BufferCreateFlags(), size, bufuse },
			&mreq, &buf, &alloc, &alli);
		if (persistent_map) **persistent_map = alli.pMappedData;
		assert(res == VK_SUCCESS);
	}

	void* buffer::map() {
		void* data;
		auto res = vmaMapMemory(dev->allocator, alloc, &data);
		assert(res == VK_SUCCESS);
		return data;
	}

	void buffer::unmap() {
		vmaUnmapMemory(dev->allocator, alloc);
	}

	buffer::~buffer() {
		vmaDestroyBuffer(dev->allocator, buf, alloc);
	}

	image::image(device * dev, vk::ImageCreateFlags flg, vk::ImageType type, vk::Extent3D size, vk::Format fmt, vk::ImageTiling til, vk::ImageUsageFlags use, vk::MemoryPropertyFlags memuse,
			size_t mip_count, size_t array_layers,
			optional<vk::UniqueImageView*> iv, vk::ImageViewType iv_type, vk::ImageSubresourceRange iv_sr) :dev(dev) {
		VmaMemoryRequirements mreq = {};
		mreq.requiredFlags = (VkMemoryPropertyFlags)memuse;
		VmaAllocationInfo alli;
		auto res = vmaCreateImage(dev->allocator, (VkImageCreateInfo*)&vk::ImageCreateInfo {
			flg, type, fmt, size, mip_count, array_layers, 
			vk::SampleCountFlagBits::e1, til, use
		}, &mreq, &img, &alloc, &alli);
		assert(res == VK_SUCCESS);
		if (iv) {
			**iv = dev->dev->createImageViewUnique(vk::ImageViewCreateInfo(vk::ImageViewCreateFlags(), vk::Image(img),
				iv_type, fmt,
				vk::ComponentMapping(), iv_sr));
		}
	}
	image::~image() {
		vmaDestroyImage(dev->allocator, img, alloc);
	}
}