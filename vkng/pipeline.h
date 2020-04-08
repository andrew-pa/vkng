#pragma once
#include "cmmn.h"
#include "device.h"

namespace vkng {

	class shader_cache {
		std::map<std::string, vk::UniqueShaderModule> shaders;
		device* dev; //this device must live at least as long as the shader_cache, RIP no Rust 😟
	public:
		shader_cache(device* dev) : dev(dev) {}

		result<vk::ShaderModule, errno_t> load_shader(const std::string& path) {
			auto f = shaders.find(path);
			if (f != shaders.end()) return f->second.get();
			else {
				std::ifstream file(path, std::ios::ate | std::ios::binary);
				if (!file) return result<vk::ShaderModule, errno_t>(1);
				auto size = (size_t)file.tellg();
				std::vector<char> buffer(size);
				file.seekg(0);
				file.read(buffer.data(), buffer.size());
				file.close();
				vk::ShaderModuleCreateInfo cfo;
				cfo.codeSize = buffer.size();
				cfo.pCode = (uint32_t*)buffer.data();
				shaders[path] = dev->dev->createShaderModuleUnique(cfo);
				return shaders[path].get();
			}
		}
	};
}
