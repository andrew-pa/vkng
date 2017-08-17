#include "renderer.h"

namespace vkng {
	namespace renderer {
		renderer::renderer(device* dev, swap_chain* swch, shader_cache* shc, camera* cam, const vector<object_desc>& od) : dev(dev), shc(shc), cam(cam) {
			objects.resize(od.size());

			//count objects to ascertain resource requirements
			size_t total_vertices = 0, total_indices = 0;
			for (size_t i = 0; i < objects.size(); ++i) {
				total_vertices += od[i].vertices.size();
				total_indices += objects[i].index_count = od[i].indices.size();
			}
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

			//create uniform buffer
			vk::PhysicalDeviceProperties props;
			dev->pdevice.getProperties(&props); // query for how close together we can put the buffers
			auto ubuf_aligned_size = props.limits.minUniformBufferOffsetAlignment;

			ubuf = make_unique<buffer>(dev, ubuf_aligned_size*objects.size(), vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				make_optional((void**)&ubuf_map));

			//create descriptor stuff for objects
			// - pool
			vk::DescriptorPoolSize pool_sizes[] = {
				{ vk::DescriptorType::eUniformBuffer, objects.size() },
				{ vk::DescriptorType::eCombinedImageSampler, objects.size() },
			};
			obj_desc_pool = dev->dev->createDescriptorPoolUnique(vk::DescriptorPoolCreateInfo{
				vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, objects.size(), 2, pool_sizes
			});
			// - layout
			obj_desc_layout = dev->create_desc_set_layout({
				vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
				vk::DescriptorSetLayoutBinding{1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment}
			});
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
			dev->dev->updateDescriptorSets(writes, {});

			//create pipeline layouts
			smp_pl_layout = dev->dev->createPipelineLayoutUnique(vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(), 1, &obj_desc_layout.get(),
				1, &vk::PushConstantRange{vk::ShaderStageFlagBits::eVertex, 0, sizeof(mat4)} //push constant for view_proj
			});



			//create swap chain resources (pipelines, framebuffers)
			create(swch);

			dev->graphics_qu.waitIdle();
		}

		void renderer::create(swap_chain* swch) {
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
			array<vk::AttachmentReference, gbuf_count> gbuf_write_ref, gbuf_read_ref;
			for (size_t i = 0; i < gbuf_count; ++i) {
				attachments.push_back({ vk::AttachmentDescriptionFlags(), //gbuffer
								vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
								vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
								vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eColorAttachmentOptimal });
				gbuf_write_ref[i] = vk::AttachmentReference { attachments.size()-1, vk::ImageLayout::eColorAttachmentOptimal };
				gbuf_read_ref[i] = vk::AttachmentReference { attachments.size()-1, vk::ImageLayout::eShaderReadOnlyOptimal };
			}
			vk::AttachmentReference dep_ref{ 1, vk::ImageLayout::eDepthStencilAttachmentOptimal };
			vector<vk::SubpassDescription> subpasses = {
				// render into g-buffer
				{ vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, 0, nullptr, gbuf_count, gbuf_write_ref.data(), nullptr, &dep_ref },
				// do lighting and render to backbuffer
				{ vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, gbuf_count, gbuf_read_ref.data(), 1, &col_ref, nullptr, nullptr }
			};
			vector<vk::SubpassDependency> dpnd = {
				// transition g-buffer to ColorAttachmentOutput from eShaderRead
				{ VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eColorAttachmentOutput, 
					vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eColorAttachmentRead|vk::AccessFlagBits::eColorAttachmentWrite },
				// transition g-buffer to eShaderRead from ColorAttachmentOutput
				{ 0, 1, vk::PipelineStageFlagBits::eBottomOfPipe, vk::PipelineStageFlagBits::eFragmentShader, 
					vk::AccessFlagBits::eColorAttachmentRead|vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eInputAttachmentRead|vk::AccessFlagBits::eShaderRead },
				// transition backbuffer to ColorAttachmentOutput from ColorAttachmentRead
				{ 0, 1, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
					vk::AccessFlagBits::eColorAttachmentRead, vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite },
			};

			vk::RenderPassCreateInfo rpcfo{ vk::RenderPassCreateFlags(),
				attachments.size(), attachments.data(), subpasses.size(), subpasses.data(), dpnd.size(), dpnd.data() };
			smp_rp = dev->dev->createRenderPassUnique(rpcfo);

			VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = (VkShaderModule)shc->load_shader("shader.vert.spv");
			vertShaderStageInfo.pName = "main";

			VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = (VkShaderModule)shc->load_shader("gbuffer.frag.spv");
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

			cmd_bufs = dev->alloc_cmd_buffers(swch->images.size());
			extent = swch->extent;

			//create G-Buffer
			gbuf_imv.resize(gbuf_count);
			for (size_t i = 0; i < gbuf_count; ++i) {
				// create the images & image views
				gbuf_img.push_back(make_unique<image>(
					dev, vk::ImageType::e2D, vk::Extent3D{extent.width, extent.height, 1}, vk::Format::eR32G32B32A32Sfloat,
					vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
					vk::MemoryPropertyFlagBits::eDeviceLocal, &gbuf_imv[i], vk::ImageViewType::e2D,
					vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
				));
			}
			fb = swch->create_framebuffers(smp_rp.get(), [&](size_t, vector<vk::ImageView>& att) {
				for (const auto& iv : gbuf_imv) att.push_back(iv.get());
			});

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
			vk::ClearValue cc[] = {
							vk::ClearColorValue{ array<float,4>{0.1f, 0.1f, 0.1f, 1.f} },
							vk::ClearDepthStencilValue{1.f, 0}
			};
			auto rbio = vk::RenderPassBeginInfo{ smp_rp.get(), fb[image_index].get(),
				vk::Rect2D(vk::Offset2D(), extent), 2, cc };
			cb->beginRenderPass(rbio, vk::SubpassContents::eInline);

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

			cb->endRenderPass();
			cb->end();
			return cb.get();
		}

	}
}
