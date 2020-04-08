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
	Objects consist of a model (vertices<p/n/t/t>,indices), std::optional<texture>, transform, material params
	*/
	namespace renderer {
		struct vertex {
			vec3 pos, norm, tang;
			vec2 tex_coord;
			static vk::VertexInputBindingDescription binding_desc() {
				return vk::VertexInputBindingDescription{ 0, sizeof(vertex) };
			}
			static std::array<vk::VertexInputAttributeDescription, 4> attrib_desc() {
				return {
					vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, offsetof(vertex, pos)},
					vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32Sfloat, offsetof(vertex, norm)},
					vk::VertexInputAttributeDescription{2, 0, vk::Format::eR32G32B32Sfloat, offsetof(vertex, tang)},
					vk::VertexInputAttributeDescription{3, 0, vk::Format::eR32G32Sfloat, offsetof(vertex, tex_coord)},
				};
			}
		};
		
		struct material {
			vec4 base_color;
			float roughness, metallic;
			vec2 padding;
			material(vec3 base, float r, float m = 0.02) : base_color(base, 1.f), roughness(r), metallic(m) {}
		};

		struct object_desc {
			std::vector<vertex> vertices;
			std::vector<uint32> indices;
			mat4 transform;
			vk::ImageView diffuse_texture;
			material mat;
		};

		struct object {
			uint32_t index_count;
			vk::DeviceSize vertex_offset, index_offset;
			vk::UniqueDescriptorSet descriptor_set;
			mat4* transform;
			material* mat;
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

			std::vector<std::unique_ptr<image>> gbuf_img;
			std::unique_ptr<image> itrmd_img; // an intermediate buffer to hold the summed final colors of objects

			std::vector<vk::UniqueImageView> gbuf_imv;
			vk::UniqueImageView itrmd_imv;

			vk::UniqueSampler fsmp, nsmp;

			vk::UniqueDescriptorPool obj_desc_pool, aux_desc_pool;
			vk::UniqueDescriptorSetLayout obj_desc_layout, postprocess_desc_layout, light_desc_layout;
			vk::DescriptorSet postprocess_desc, light_desc, skybox_desc;

			std::unique_ptr<buffer> vxbuf, ixbuf, ubuf;
			void* ubuf_map;
			std::vector<object> objects;

			vk::DeviceSize box_vertex_offset, box_index_offset;
			
			vk::UniquePipelineLayout smp_pl_layout, sky_light_pl_layout, skybox_pl_layout, postprocess_pl_layout;
			vk::UniquePipeline gbuf_pl, sky_light_pl, skybox_pl, postprocess_pl;

			vk::UniqueRenderPass smp_rp;

			std::vector<vk::UniqueCommandBuffer> cmd_bufs;
			std::vector<vk::UniqueFramebuffer> fb;
			vk::Extent2D extent;

			camera* cam;

			renderer(device* dev, swap_chain* swch, shader_cache* shc,
				camera* cam, const std::vector<object_desc>& objects, vk::ImageView sky);

			void reset();
			void recreate(swap_chain*);

			vk::CommandBuffer render(uint32_t image_index);
		private:
			void create(swap_chain*);
			void create_render_pass(swap_chain*);
			void create_pipelines(swap_chain*);
			void create_framebuffers(swap_chain*);
		};
	}
}
