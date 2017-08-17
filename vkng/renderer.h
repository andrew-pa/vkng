#pragma once
#include "cmmn.h"
#include "device.h"
#include "swap_chain.h"
#include "pipeline.h"
#include "camera.h"

namespace vkng {
	/*
	Renderer
	draw objects
	Objects consist of a model (vertices<p/n/t/t>,indices), optional<texture>, transform, material params
	*/
	namespace renderer {
		struct vertex {
			vec3 pos, norm, tang;
			vec2 tex_coord;
			static vk::VertexInputBindingDescription binding_desc() {
				return vk::VertexInputBindingDescription{ 0, sizeof(vertex) };
			}
			static array<vk::VertexInputAttributeDescription, 4> attrib_desc() {
				return {
					vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, offsetof(vertex, pos)},
					vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32Sfloat, offsetof(vertex, norm)},
					vk::VertexInputAttributeDescription{2, 0, vk::Format::eR32G32B32Sfloat, offsetof(vertex, tang)},
					vk::VertexInputAttributeDescription{3, 0, vk::Format::eR32G32Sfloat, offsetof(vertex, tex_coord)},
				};
			}
		};
		
		struct object_desc {
			vector<vertex> vertices;
			vector<uint32> indices;
			mat4 transform;
			vk::ImageView diffuse_texture;
		};

		struct object {
			uint32_t index_count;
			vk::DeviceSize vertex_offset, index_offset;
			vk::UniqueDescriptorSet descriptor_set;
			mat4* transform;
		};


		/*
			render pass structure

			-draw stuff> gbuffer[4] & depth buffer
			gbuffer[4] -fsq> backbuffer
		*/

		const size_t gbuf_count = 4;
		struct renderer {
			device* dev;
			shader_cache* shc;

			vector<unique_ptr<image>> gbuf_img;
			vector<vk::UniqueImageView> gbuf_imv;
			vk::UniqueRenderPass gbuf_rp;

			vk::UniqueSampler fsmp;

			vk::UniqueDescriptorPool obj_desc_pool;
			vk::UniqueDescriptorSetLayout obj_desc_layout;
			unique_ptr<buffer> vxbuf, ixbuf, ubuf;
			void* ubuf_map;
			vector<object> objects;
			
			vk::UniquePipelineLayout smp_pl_layout;
			vk::UniquePipeline gbuf_pl;

			vk::UniqueRenderPass smp_rp;

			vector<vk::UniqueCommandBuffer> cmd_bufs;
			vector<vk::UniqueFramebuffer> fb;
			vk::Extent2D extent;

			camera* cam;

			renderer(device* dev, swap_chain* swch, shader_cache* shc, camera* cam, const vector<object_desc>& objects);

			void reset();
			void recreate(swap_chain*);

			vk::CommandBuffer render(uint32_t image_index);
		private:
			void create(swap_chain*);
		};
	}
}
