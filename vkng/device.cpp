#include "device.h"
#include <set>

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
		dcfo.pEnabledFeatures = &devfeat;
		dev = pdevice.createDeviceUnique(dcfo);

		graphics_qu = dev->getQueue(qu_fam.graphics, 0);
		present_qu = dev->getQueue(qu_fam.present, 0);
	}

	device::~device() {
		
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

}