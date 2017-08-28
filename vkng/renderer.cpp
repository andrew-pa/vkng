#include "renderer.h"

namespace vkng {
	namespace renderer {

		void generate_cube(float width, float height, float depth, function<void(vec3, vec3, vec3, vec2)> vertex, function<void(size_t)> index) {

			float w2 = 0.5f*width;
			float h2 = 0.5f*height;
			float d2 = 0.5f*depth;

			// Fill in the front face vertex data.
			vertex({ -w2, -h2, -d2 }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f });
			vertex({-w2, +h2, -d2},{ 0.0f, 0.0f, -1.0f},{ 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f});
			vertex({+w2, +h2, -d2},{ 0.0f, 0.0f, -1.0f},{ 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f});
			vertex({+w2, -h2, -d2},{ 0.0f, 0.0f, -1.0f},{ 1.0f, 0.0f, 0.0f}, {1.0f, 1.0f});
			// F{ill in the ba}c{k face vertex data}.{}{}1
			vertex({-w2, -h2, +d2},{ 0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f});
			vertex({+w2, -h2, +d2},{ 0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f});
			vertex({+w2, +h2, +d2},{ 0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f});
			vertex({-w2, +h2, +d2},{ 0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f});
			// F{ill in the to}p{ face vertex data.}{}{}1
			vertex({-w2, +h2, -d2},{ 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f,}, {0.0f, 1.0f});
			vertex({-w2, +h2, +d2},{ 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f,}, {0.0f, 0.0f});
			vertex({+w2, +h2, +d2},{ 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f,}, {1.0f, 0.0f});
			vertex({+w2, +h2, -d2},{ 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f,}, {1.0f, 1.0f});
			// F{ill in the bo}t{tom face vertex da}t{a.}{}1
			vertex({-w2, -h2, -d2},{ 0.0f, -1.0f, 0.0f},{ -1.0f, 0.0f, 0.0f},{ 1.0f, 1.0f});
			vertex({+w2, -h2, -d2},{ 0.0f, -1.0f, 0.0f},{ -1.0f, 0.0f, 0.0f},{ 0.0f, 1.0f});
			vertex({+w2, -h2, +d2},{ 0.0f, -1.0f, 0.0f},{ -1.0f, 0.0f, 0.0f},{ 0.0f, 0.0f});
			vertex({-w2, -h2, +d2},{ 0.0f, -1.0f, 0.0f},{ -1.0f, 0.0f, 0.0f},{ 1.0f, 0.0f});
			// F{ill in the le}f{t face vertex data}.{}{}1
			vertex({-w2, -h2, +d2},{ -1.0f, 0.0f, 0.0f},{ 0.0f, 0.0f, -1.0f},{ 0.0f, 1.0f});
			vertex({-w2, +h2, +d2},{ -1.0f, 0.0f, 0.0f},{ 0.0f, 0.0f, -1.0f},{ 0.0f, 0.0f});
			vertex({-w2, +h2, -d2},{ -1.0f, 0.0f, 0.0f},{ 0.0f, 0.0f, -1.0f},{ 1.0f, 0.0f});
			vertex({-w2, -h2, -d2},{ -1.0f, 0.0f, 0.0f},{ 0.0f, 0.0f, -1.0f},{ 1.0f, 1.0f});
			// F{ill in the ri}g{ht face vertex dat}a{.}{}1
			vertex({+w2, -h2, -d2},{ 1.0f, 0.0f, 0.0f,}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f});
			vertex({+w2, +h2, -d2},{ 1.0f, 0.0f, 0.0f,}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f});
			vertex({+w2, +h2, +d2},{ 1.0f, 0.0f, 0.0f,}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f});
			vertex({+w2, -h2, +d2},{ 1.0f, 0.0f, 0.0f,}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f});

			// Fill in the front face index data
			index(0); index(1); index(2);
			index(0); index(2); index(3);

			// Fill in the back face index data
			index(4); index(5); index(6);
			index(4); index(6); index(7);

			// Fill in the top face index data
			index(8); index(9); index(10);
			index(8); index(10); index(11);

			// Fill in the bottom face index data
			index(12); index(13); index(14);
			index(12); index(14); index(15);

			// Fill in the left face index data
			index(16); index(17); index(18);
			index(16); index(18); index(19);

			// Fill in the right face index data
			index(20); index(21); index(22);
			index(20); index(22); index(23);
		}

		renderer::renderer(device* dev, swap_chain* swch, shader_cache* shc, camera* cam,
			const vector<object_desc>& od, vk::ImageView sky_view) : dev(dev), shc(shc), cam(cam)
		{
			objects.resize(od.size());

			//count objects to ascertain resource requirements
			size_t total_vertices = 0, total_indices = 0;
			for (size_t i = 0; i < objects.size(); ++i) {
				total_vertices += od[i].vertices.size();
				total_indices += objects[i].index_count = od[i].indices.size();
			}
			total_vertices += 24; total_indices += 36; //sky box geometry
			total_vertices *= sizeof(vertex);
			total_indices *= sizeof(uint32);

			//create buffers
			// - create staging buffer
			auto stg_buf = buffer(dev, total_vertices + total_indices, vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

			// - copy data into staging buffer from RAM
			auto data = (char*)stg_buf.map();
			size_t voffset = 0, ioffset = 0;
			for (size_t i = 0; i < objects.size(); ++i) {
				objects[i].vertex_offset = voffset;
				memcpy(data + voffset, od[i].vertices.data(), sizeof(vertex)*od[i].vertices.size());
				voffset += sizeof(vertex)*od[i].vertices.size();

				objects[i].index_offset = ioffset;
				memcpy(data + total_vertices + ioffset, od[i].indices.data(), sizeof(uint32)*od[i].indices.size());
				ioffset += sizeof(uint32)*od[i].indices.size();
			}

			// generate skybox geometry in-place
			box_vertex_offset = voffset; box_index_offset = ioffset;
			generate_cube(4000.f, 4000.f, 4000.f, [&](vec3 p, vec3 n, vec3 tn, vec2 tx) {
				*(vertex*)(data + voffset) = vertex{ p, n, tn, tx };
				voffset += sizeof(vertex);
			}, [&](size_t ix) {
				*(uint32*)(data + total_vertices + ioffset) = ix;
				ioffset += sizeof(uint32);
			});

			stg_buf.unmap();

			// - create vertex/index buffers
			vxbuf = make_unique<buffer>(dev, total_vertices,
				vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
				vk::MemoryPropertyFlagBits::eDeviceLocal);
			ixbuf = make_unique<buffer>(dev, total_indices,
				vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
				vk::MemoryPropertyFlagBits::eDeviceLocal);

			// - create command buffer to copy from staging ⇒ vertex/index buffers
			auto scb = move(dev->alloc_cmd_buffers()[0]);
			scb->begin(vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

			scb->copyBuffer(stg_buf, vxbuf->operator vk::Buffer(), { vk::BufferCopy{0, 0, total_vertices } });
			scb->copyBuffer(stg_buf, ixbuf->operator vk::Buffer(), { vk::BufferCopy{total_vertices, 0, total_indices } });

			scb->end();
			dev->graphics_qu.submit({ vk::SubmitInfo{0, nullptr, nullptr, 1, &scb.get()} }, nullptr); // start actually copying stuff while we create everything else

			//create samplers
			fsmp = dev->dev->createSamplerUnique(vk::SamplerCreateInfo{
					vk::SamplerCreateFlags(), vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eNearest, vk::SamplerAddressMode::eRepeat,
					vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, 0.f, VK_TRUE, 16.f
			});
			nsmp = dev->dev->createSamplerUnique(vk::SamplerCreateInfo{
					vk::SamplerCreateFlags(), vk::Filter::eNearest, vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest, vk::SamplerAddressMode::eRepeat,
					vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, 0.f, VK_FALSE, 0.f
			});

			//create uniform buffer
			vk::PhysicalDeviceProperties props;
			dev->pdevice.getProperties(&props); // query for how close together we can put the buffers
			auto ubuf_aligned_size = props.limits.minUniformBufferOffsetAlignment;

			ubuf = make_unique<buffer>(dev, ubuf_aligned_size*objects.size(), vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				make_optional((void**)&ubuf_map));

			//create descriptor pools, layouts & sets 
			// - pool
			vk::DescriptorPoolSize pool_sizes[] = {
				{ vk::DescriptorType::eUniformBuffer, objects.size() },
				{ vk::DescriptorType::eCombinedImageSampler, objects.size() },
			};
			obj_desc_pool = dev->dev->createDescriptorPoolUnique(vk::DescriptorPoolCreateInfo{
				vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, objects.size(), 2, pool_sizes
			});
			
			pool_sizes[0] = { vk::DescriptorType::eCombinedImageSampler, gbuf_count+2 };
			aux_desc_pool = dev->dev->createDescriptorPoolUnique(vk::DescriptorPoolCreateInfo{
				vk::DescriptorPoolCreateFlags(), 3, 1, pool_sizes
			});

			// - layout
			obj_desc_layout = dev->create_desc_set_layout({
				vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
				vk::DescriptorSetLayoutBinding{1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment}
			});
			light_desc_layout = dev->create_desc_set_layout({
				vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, gbuf_count, vk::ShaderStageFlagBits::eFragment},
			});
			postprocess_desc_layout = dev->create_desc_set_layout({
				vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
			});
			// we can reuse the postprocess_desc_layout for the skybox for now because they both have the same layout (just sampling one texture)

			// - sets
			vector<vk::DescriptorSetLayout> layouts(objects.size(), obj_desc_layout.get());
			auto sets = dev->dev->allocateDescriptorSetsUnique(vk::DescriptorSetAllocateInfo{
				obj_desc_pool.get(), objects.size(), layouts.data()
			});
			vector<vk::DescriptorBufferInfo> ubuf_ifo(objects.size());
			vector<vk::DescriptorImageInfo> dftx_ifo(objects.size());
			vector<vk::WriteDescriptorSet> writes;
			for (size_t i = 0; i < objects.size(); ++i) {
				objects[i].transform = (mat4*)(((char*)ubuf_map) + i*ubuf_aligned_size);
				*objects[i].transform = od[i].transform;
				objects[i].descriptor_set = move(sets[i]);
				ubuf_ifo[i] = vk::DescriptorBufferInfo{ ubuf->operator vk::Buffer(), i*ubuf_aligned_size, sizeof(mat4) };
				writes.push_back(vk::WriteDescriptorSet{ objects[i].descriptor_set.get(), 0, 0, 1,
					vk::DescriptorType::eUniformBuffer, nullptr, &ubuf_ifo[i] });
				dftx_ifo[i] = vk::DescriptorImageInfo{ fsmp.get(), od[i].diffuse_texture, vk::ImageLayout::eShaderReadOnlyOptimal };
				writes.push_back(vk::WriteDescriptorSet{ objects[i].descriptor_set.get(), 1, 0, 1,
					vk::DescriptorType::eCombinedImageSampler, &dftx_ifo[i] });
			}

			layouts = vector<vk::DescriptorSetLayout>{ postprocess_desc_layout.get(), postprocess_desc_layout.get(), light_desc_layout.get() };
			auto aux_desc = dev->dev->allocateDescriptorSets(vk::DescriptorSetAllocateInfo{
				aux_desc_pool.get(), 3, layouts.data()
			});
			postprocess_desc = aux_desc[0];
			skybox_desc = aux_desc[1];
			light_desc = aux_desc[2];

			auto sky_info = vk::DescriptorImageInfo{ fsmp.get(), sky_view,
				vk::ImageLayout::eShaderReadOnlyOptimal };
			writes.push_back(vk::WriteDescriptorSet{ skybox_desc, 0, 0, 1,
				vk::DescriptorType::eCombinedImageSampler, &sky_info });

			dev->dev->updateDescriptorSets(writes, {});

			//create pipeline layouts
			smp_pl_layout = dev->dev->createPipelineLayoutUnique(vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(), 1, &obj_desc_layout.get(),
				1, &vk::PushConstantRange{vk::ShaderStageFlagBits::eVertex, 0, sizeof(mat4)} //push constant for view_proj
			});
			light_pl_layout = dev->dev->createPipelineLayoutUnique(vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(), 1, &light_desc_layout.get(),
			});
			skybox_pl_layout = dev->dev->createPipelineLayoutUnique(vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(), 1, &postprocess_desc_layout.get(),
				1, &vk::PushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(mat4)) //push constant for view_proj like above
			});
			postprocess_pl_layout = dev->dev->createPipelineLayoutUnique(vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(), 1, &postprocess_desc_layout.get(),
			});


			//create swap chain resources (pipelines, framebuffers)
			create(swch);

			dev->graphics_qu.waitIdle();
		}
		
		void renderer::create_render_pass(swap_chain* swch) {
			vector<vk::AttachmentDescription> attachments = {
					{ vk::AttachmentDescriptionFlags(), //swapchain color
								swch->format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
								vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
								vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR },

					{ vk::AttachmentDescriptionFlags(), //depth
								vk::Format::eD32Sfloat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
								vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
								vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal }
			};
			vk::AttachmentReference col_ref{ 0, vk::ImageLayout::eColorAttachmentOptimal };
			vk::AttachmentReference dep_ref{ 1, vk::ImageLayout::eDepthStencilAttachmentOptimal };

			attachments.push_back({ vk::AttachmentDescriptionFlags(), //intermediate buffer
							vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
							vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
							vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eColorAttachmentOptimal });
			vk::AttachmentReference itrmd_write_ref{ 2, vk::ImageLayout::eColorAttachmentOptimal };
			vk::AttachmentReference itrmd_read_ref{ 2, vk::ImageLayout::eShaderReadOnlyOptimal };

			array<vk::AttachmentReference, gbuf_count> gbuf_write_ref, gbuf_read_ref;
			for (size_t i = 0; i < gbuf_count; ++i) {
				attachments.push_back({ vk::AttachmentDescriptionFlags(), //gbuffer
								vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
								vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
								vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eColorAttachmentOptimal });
				gbuf_write_ref[i] = vk::AttachmentReference{ attachments.size() - 1, vk::ImageLayout::eColorAttachmentOptimal };
				gbuf_read_ref[i] = vk::AttachmentReference{ attachments.size() - 1, vk::ImageLayout::eShaderReadOnlyOptimal };
			}

			vector<vk::SubpassDescription> subpasses = {
				// render into g-buffer
				{ vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, 0, nullptr, gbuf_count, gbuf_write_ref.data(), nullptr, &dep_ref },
				// do lighting and write to itermediate buffer
				{ vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, gbuf_count, gbuf_read_ref.data(), 1, &itrmd_write_ref, nullptr, &dep_ref },
				// postprocess and write to backbuffer
				{ vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, 1, &itrmd_read_ref, 1, &col_ref, nullptr, nullptr }
			};
			vector<vk::SubpassDependency> dpnd = {
				// transition g-buffer to ColorAttachmentOutput from eShaderRead
				{ VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eColorAttachmentOutput,
					vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite },
				// transition g-buffer to eShaderRead from ColorAttachmentOutput
				{ 0, 1, vk::PipelineStageFlagBits::eBottomOfPipe, vk::PipelineStageFlagBits::eFragmentShader,
					vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eShaderRead },

				// transition intermediate buffer to ColorAttachmentOutput from eShaderRead
				{ 0, 1, vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eColorAttachmentOutput,
					vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite },
				// transition intermediate buffer to eShaderRead from ColorAttachmentOutput
				{ 1, 2, vk::PipelineStageFlagBits::eBottomOfPipe, vk::PipelineStageFlagBits::eFragmentShader,
					vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eShaderRead },

				// transition backbuffer to ColorAttachmentOutput from ColorAttachmentRead
				{ 1, 2, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
					vk::AccessFlagBits::eColorAttachmentRead, vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite },
			};

			vk::RenderPassCreateInfo rpcfo{ vk::RenderPassCreateFlags(),
				attachments.size(), attachments.data(), subpasses.size(), subpasses.data(), dpnd.size(), dpnd.data() };
			smp_rp = dev->dev->createRenderPassUnique(rpcfo);
		}

		void renderer::create_pipelines(swap_chain* swch) {
			VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = (VkShaderModule)shc->load_shader("shader.vert.spv").unwrap();
			vertShaderStageInfo.pName = "main";

			VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = (VkShaderModule)shc->load_shader("gbuffer.frag.spv").unwrap();
			fragShaderStageInfo.pName = "main";

			VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

			VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			auto bd = vertex::binding_desc(); auto ad = vertex::attrib_desc();
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.pVertexBindingDescriptions = (VkVertexInputBindingDescription*)&bd;

			vertexInputInfo.vertexAttributeDescriptionCount = ad.size();
			vertexInputInfo.pVertexAttributeDescriptions = (VkVertexInputAttributeDescription*)ad.data();

			VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)swch->extent.width;
			viewport.height = (float)swch->extent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor = {};
			scissor.offset = { 0, 0 };
			scissor.extent = swch->extent;

			VkPipelineViewportStateCreateInfo viewportState = {};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor;

			VkPipelineRasterizationStateCreateInfo rasterizer = {};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			rasterizer.depthBiasEnable = VK_FALSE;

			VkPipelineDepthStencilStateCreateInfo depthStencil = {};
			depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencil.depthTestEnable = VK_TRUE;
			depthStencil.depthWriteEnable = VK_TRUE;
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.stencilTestEnable = VK_FALSE;

			VkPipelineMultisampleStateCreateInfo multisampling = {};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;
			array<VkPipelineColorBlendAttachmentState, gbuf_count> blend_att_state;
			for (size_t i = 0; i < gbuf_count; ++i) blend_att_state[i] = colorBlendAttachment;

			VkPipelineColorBlendStateCreateInfo colorBlending = {};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = gbuf_count;
			colorBlending.pAttachments = blend_att_state.data();
			colorBlending.blendConstants[0] = 0.0f;
			colorBlending.blendConstants[1] = 0.0f;
			colorBlending.blendConstants[2] = 0.0f;
			colorBlending.blendConstants[3] = 0.0f;

			VkGraphicsPipelineCreateInfo pipelineInfo = {};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount = 2;
			pipelineInfo.pStages = shaderStages;
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pDepthStencilState = &depthStencil;
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.layout = (VkPipelineLayout)smp_pl_layout.get();
			pipelineInfo.renderPass = (VkRenderPass)smp_rp.get();
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			gbuf_pl = dev->dev->createGraphicsPipelineUnique(VK_NULL_HANDLE, pipelineInfo);

			shaderStages[0].module = (VkShaderModule)shc->load_shader("skybox.vert.spv").unwrap();
			shaderStages[1].module = (VkShaderModule)shc->load_shader("skybox.frag.spv").unwrap();
			colorBlending.attachmentCount = 1;
			depthStencil.depthTestEnable = VK_TRUE;
			depthStencil.depthWriteEnable = VK_FALSE;
			rasterizer.cullMode = VK_CULL_MODE_NONE;
			pipelineInfo.subpass = 1;
			pipelineInfo.layout = (VkPipelineLayout)skybox_pl_layout.get();
			skybox_pl = dev->dev->createGraphicsPipelineUnique(VK_NULL_HANDLE, pipelineInfo);

			vertexInputInfo.vertexAttributeDescriptionCount = 0;
			vertexInputInfo.vertexBindingDescriptionCount = 0;
			shaderStages[0].module = (VkShaderModule)shc->load_shader("fsq.vert.spv").unwrap();
			shaderStages[1].module = (VkShaderModule)shc->load_shader("directional-light.frag.spv").unwrap();
			colorBlending.attachmentCount = 1;
			depthStencil.depthTestEnable = VK_FALSE;
			depthStencil.depthWriteEnable = VK_FALSE;
			rasterizer.cullMode = VK_CULL_MODE_NONE;
			pipelineInfo.subpass = 1;
			pipelineInfo.layout = (VkPipelineLayout)light_pl_layout.get();
			directional_light_pl = dev->dev->createGraphicsPipelineUnique(VK_NULL_HANDLE, pipelineInfo);

			vertexInputInfo.vertexAttributeDescriptionCount = 0;
			vertexInputInfo.vertexBindingDescriptionCount = 0;
			shaderStages[0].module = (VkShaderModule)shc->load_shader("fsq.vert.spv").unwrap();
			shaderStages[1].module = (VkShaderModule)shc->load_shader("postprocess.frag.spv").unwrap();
			colorBlending.attachmentCount = 1;
			depthStencil.depthTestEnable = VK_FALSE;
			depthStencil.depthWriteEnable = VK_FALSE;
			rasterizer.cullMode = VK_CULL_MODE_NONE;
			pipelineInfo.subpass = 2;
			pipelineInfo.layout = (VkPipelineLayout)postprocess_pl_layout.get();
			postprocess_pl = dev->dev->createGraphicsPipelineUnique(VK_NULL_HANDLE, pipelineInfo);
		}

		void renderer::create_framebuffers(swap_chain* swch) {
			//create framebuffers
			gbuf_imv.resize(gbuf_count);
			for (size_t i = 0; i < gbuf_count; ++i) {
				// create the images & image views
				gbuf_img.push_back(make_unique<image>(
					dev, vk::ImageType::e2D, vk::Extent3D{ extent.width, extent.height, 1 }, vk::Format::eR32G32B32A32Sfloat,
					vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment,
					vk::MemoryPropertyFlagBits::eDeviceLocal, &gbuf_imv[i], vk::ImageViewType::e2D,
					vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
				));
			}
			itrmd_img = make_unique<image>(
				dev, vk::ImageType::e2D, vk::Extent3D{ extent.width, extent.height, 1 }, vk::Format::eR32G32B32A32Sfloat,
				vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment,
				vk::MemoryPropertyFlagBits::eDeviceLocal, &itrmd_imv, vk::ImageViewType::e2D,
				vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
			);
			fb = swch->create_framebuffers(smp_rp.get(), [&](size_t, vector<vk::ImageView>& att) {
				att.push_back(itrmd_imv.get());
				for (const auto& iv : gbuf_imv) att.push_back(iv.get());
			});

			vector<vk::DescriptorImageInfo> gbuf_ifo;
			for (size_t i = 0; i < gbuf_count; ++i)
				gbuf_ifo.push_back(vk::DescriptorImageInfo{ nsmp.get(), gbuf_imv[i].get(), vk::ImageLayout::eShaderReadOnlyOptimal });
			vk::DescriptorImageInfo itrmd_ifo{ nsmp.get(), itrmd_imv.get(), vk::ImageLayout::eShaderReadOnlyOptimal };
			dev->dev->updateDescriptorSets({
				vk::WriteDescriptorSet{ light_desc, 0, 0, gbuf_count, vk::DescriptorType::eCombinedImageSampler, gbuf_ifo.data() },
				vk::WriteDescriptorSet{ postprocess_desc, 0, 0, 1, vk::DescriptorType::eCombinedImageSampler, &itrmd_ifo }
			}, {});
		}

		void renderer::create(swap_chain* swch) {
			extent = swch->extent;
			create_render_pass(swch);
			create_pipelines(swch);
			create_framebuffers(swch);
			cmd_bufs = dev->alloc_cmd_buffers(swch->images.size());
			cam->update_proj(vec2(extent.width, extent.height));
		}

		void renderer::reset() {
			//release swap chain resources
			cmd_bufs.clear();
			fb.clear();
			gbuf_img.clear();
			gbuf_imv.clear();
			gbuf_pl.reset();
		}
		void renderer::recreate(swap_chain* swch) {
			//recreate them
			create(swch);
		}

		vk::CommandBuffer renderer::render(uint32_t image_index) {
			auto& cb = cmd_bufs[image_index];
			cb->begin(vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
			const vector<vk::ClearValue> cc = {
				vk::ClearColorValue{ array<float,4>{0.1f, 0.1f, 0.1f, 1.f} },
				vk::ClearDepthStencilValue{1.f, 0},
				vk::ClearColorValue{ array<float,4>{0.f, 0.f, 0.f, 0.f} },
				vk::ClearColorValue{ array<float,4>{0.f, 0.f, 0.f, 0.f} },
				vk::ClearColorValue{ array<float,4>{0.f, 0.f, 0.f, 0.f} },
				vk::ClearColorValue{ array<float,4>{0.f, 0.f, 0.f, 0.f} },
				vk::ClearColorValue{ array<float,4>{0.f, 0.f, 0.f, 0.f} },
			};
			auto rbio = vk::RenderPassBeginInfo{ smp_rp.get(), fb[image_index].get(),
				vk::Rect2D(vk::Offset2D(), extent), cc.size(), cc.data() };
			cb->beginRenderPass(rbio, vk::SubpassContents::eInline);

			// - Draw Objects -
			cb->bindPipeline(vk::PipelineBindPoint::eGraphics, gbuf_pl.get());
			cb->pushConstants<mat4>(smp_pl_layout.get(), vk::ShaderStageFlagBits::eVertex, 0, { cam->_proj * cam->_view });

			vk::Buffer bufs[] = { vxbuf->operator vk::Buffer() };
			for (const auto& o : objects) {
				cb->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, smp_pl_layout.get(), 0,
				{ o.descriptor_set.get() }, { });
				cb->bindVertexBuffers(0, 1, bufs, &o.vertex_offset);
				cb->bindIndexBuffer(ixbuf->operator vk::Buffer(), o.index_offset, vk::IndexType::eUint32);
				cb->drawIndexed(o.index_count, 1, 0, 0, 0);
			}

			// - Do Lighting -
			cb->nextSubpass(vk::SubpassContents::eInline);

			// calculate directional lighting
			cb->bindPipeline(vk::PipelineBindPoint::eGraphics, directional_light_pl.get());
			cb->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, light_pl_layout.get(), 0, { light_desc }, {});
			cb->draw(3, 1, 0, 0);

			// draw background with skybox
			cb->bindPipeline(vk::PipelineBindPoint::eGraphics, skybox_pl.get());
			cb->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, skybox_pl_layout.get(), 0, { skybox_desc }, {});
			mat4 skycam_view = cam->_view;
			skycam_view[3][0] = 0.f;
			skycam_view[3][1] = 0.f;
			skycam_view[3][2] = 0.f;
			cb->pushConstants<mat4>(skybox_pl_layout.get(), vk::ShaderStageFlagBits::eVertex, 0, { cam->_proj * skycam_view });
			cb->bindVertexBuffers(0, 1, bufs, &box_vertex_offset);
			cb->bindIndexBuffer(ixbuf->operator vk::Buffer(), box_index_offset, vk::IndexType::eUint32);
			cb->drawIndexed(36, 1, 0, 0, 0);

			// - Postprocessing -
			cb->nextSubpass(vk::SubpassContents::eInline);

			cb->bindPipeline(vk::PipelineBindPoint::eGraphics, postprocess_pl.get());
			cb->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, postprocess_pl_layout.get(), 0, { postprocess_desc }, {});
			cb->draw(3, 1, 0, 0);

			cb->endRenderPass();
			cb->end();
			return cb.get();
		}

	}
}
