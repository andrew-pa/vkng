#pragma once
#include "cmmn.h"
#include "device.h"
#include "swap_chain.h"
#include "pipeline.h"

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
			vector<vertex>* vertices;
			vector<uint32>* indices;
			mat4 transform;
			vk::ImageView diffuse_texture;
		};

		struct object {
			uint32_t index_count;
			vk::DeviceSize vertex_offset, index_offset;
			vk::UniqueDescriptorSet descriptor_set;
			mat4* transform;
		};

		struct camera {
			mat4 proj; vec3 pos, targ, up;
			mat4 view_proj() {
				return proj * lookAt(pos, targ, up);
			}

			void update_proj(vk::Extent2D extent) {
				proj = perspective(pi<float>() / 2.f, extent.width / (float)extent.height, 0.01f, 1000.f);
				proj[1][1] *= -1;
			}

			camera(vec3 p = vec3(0.f, 1.f, 2.f), vec3 t = vec3(0.f), vec3 u = vec3(0.f, 0.f, 1.f)) : pos(p), targ(t), up(u) { }
		};

		struct renderer {
			device* dev;
			shader_cache* shc;
			/*vector<image> gbuf_img;
			vector<vk::UniqueImageView> gbuf_imv;
			vector<vk::UniqueFramebuffer> gbuf_fb;*/
			vk::UniqueSampler fsmp;

			vk::UniqueDescriptorPool obj_desc_pool;
			vk::UniqueDescriptorSetLayout obj_desc_layout;
			unique_ptr<buffer> vxbuf, ixbuf, ubuf;
			void* ubuf_map;
			vector<object> objects;
			
			vk::UniquePipelineLayout smp_pl_layout;
			vk::UniquePipeline smp_pl;

			vk::UniqueRenderPass smp_rp;

			vector<vk::UniqueCommandBuffer> cmd_bufs;
			vector<vk::UniqueFramebuffer> sw_fb;
			vk::Extent2D extent;

			camera cam;

			renderer(device* dev, swap_chain* swch, shader_cache* shc, const vector<object_desc>& objects);

			void reset();
			void recreate(swap_chain*);

			vk::CommandBuffer render(uint32_t image_index);
		private:
			void create(swap_chain*);
		};
	}
}
