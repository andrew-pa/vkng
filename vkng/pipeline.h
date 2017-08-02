#pragma once
#include "cmmn.h"
#include "device.h"

namespace vkng {

	class shader_cashe {
		map<string, vk::UniqueShaderModule> shaders;
		device* dev; //this device must live at least as long as the shader_cache, RIP no Rust 😟
	public:
		shader_cashe(device* dev) : dev(dev) {}

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

	struct pipeline {
		struct options {
			vk::PipelineVertexInputStateCreateInfo vi;
			vk::PipelineInputAssemblyStateCreateInfo ia;
			vk::Viewport viewport_;
			vk::Rect2D scissor;
			vk::PipelineRasterizationStateCreateInfo rs;
			vk::PipelineMultisampleStateCreateInfo multisample_ifo;
			vector<vk::PipelineColorBlendAttachmentState> blend_states;
			vk::PipelineColorBlendStateCreateInfo blend_;
			vector<vk::DynamicState> dynamic_states_;
			vector<vk::PipelineShaderStageCreateInfo> shaders;
			vector<string> shader_names;

			options() {}

			options& viewport(vec2 viewport_size) {
				viewport_ = vk::Viewport(0.f, 0.f, viewport_size.x, viewport_size.y, 0.f, 1.f);
				scissor = vk::Rect2D(vk::Offset2D(), vk::Extent2D(viewport_size.x, viewport_size.y));
				return *this;
			}
			options& input_assembly(vk::PrimitiveTopology topology, bool prim_restart = false) {
				ia.topology = topology;
				ia.primitiveRestartEnable = prim_restart;
				return *this;
			}
			options& viewport(vk::Viewport vp) {
				viewport_ = vp;
				scissor = vk::Rect2D(vk::Offset2D(), vk::Extent2D(vp.width, vp.height));
				return *this;
			}
			options& rasterizer(vk::PolygonMode poly_mode = vk::PolygonMode::eFill,
					vk::CullModeFlags cull_mode = vk::CullModeFlags(vk::CullModeFlagBits::eBack), vk::FrontFace ff = vk::FrontFace::eClockwise) {
				rs.polygonMode = poly_mode;
				rs.cullMode = cull_mode;
				rs.frontFace = ff;
				return *this;
			}
			options& rasterizer(vk::PipelineRasterizationStateCreateInfo ifo) { rs = ifo; return *this; }
			options& blend(vk::PipelineColorBlendStateCreateInfo ifo, vector<vk::PipelineColorBlendAttachmentState> states) {
				blend_ = ifo;
				blend_states = states;
				blend_.attachmentCount = blend_states.size();
				blend_.pAttachments = blend_states.data();
				return *this;
			}
			options& dynamic_states(vector<vk::DynamicState> states) {
				dynamic_states_ = states;
				return *this;
			}

			options& shader(vk::ShaderModule mod, vk::ShaderStageFlagBits stage, const string& name = "main") {
				auto ni = shader_names.size();
				shader_names.push_back(name);
				shaders.push_back(vk::PipelineShaderStageCreateInfo {
					vk::PipelineShaderStageCreateFlags(), stage, mod, shader_names[ni].c_str() });
				return *this;
			}
			options& vertex_shader(vk::ShaderModule mod, const string& name = "main") {
				return shader(mod, vk::ShaderStageFlagBits::eVertex);
			}
			options& fragment_shader(vk::ShaderModule mod, const string& name = "main") {
				return shader(mod, vk::ShaderStageFlagBits::eFragment);
			}
		};
		vk::UniquePipeline pp;
		pipeline() {}
		pipeline(device* d, vk::PipelineLayout layout, vk::RenderPass renderpass, uint32_t subpass, 
			const options& b, 
			vk::Pipeline basepipe = VK_NULL_HANDLE, int32_t baseindex = -1);
		~pipeline();
	};
}
