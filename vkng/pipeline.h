#pragma once
#include "cmmn.h"
#include "device.h"

namespace vkng {

	class shader_cache {
		map<string, vk::UniqueShaderModule> shaders;
		device* dev; //this device must live at least as long as the shader_cache, RIP no Rust 😟
	public:
		shader_cache(device* dev) : dev(dev) {}

		vk::ShaderModule load_shader(const string& path) {
			auto f = shaders.find(path);
			if (f != shaders.end()) return f->second.get();
			else {
				ifstream file(path, ios::ate | ios::binary);
				auto size = (size_t)file.tellg();
				vector<char> buffer(size);
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
