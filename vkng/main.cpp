#include "cmmn.h"
#include "app.h"
#include "device.h"
#include "swap_chain.h"
#include "pipeline.h"
#include "renderer.h"
using namespace vkng;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

/*
	PLAN
	
	Refacter some of the pipeline/render pass creation code, could be more generalized?
	Deferred Renderer
		G-Buffer
		Directional Lights/Nice Physically Based shading/Postprocess
		Shadows?
		Points lights?
	Voxels? Load scripts -> a scene? some gameplay...?
*/

void generate_torus(vec2 r, int div, function<void(vec3, vec3, vec3, vec2)> vertex, function<void(size_t)> index)
{
	int ring_count = div;
	int stack_count = div;

	vector<tuple<vec3,vec3,vec3,vec2>> frvtx;
	for (int i = 0; i < div + 1; ++i)
	{
		vec4 p = vec4(r.y, 0.f, 0.f, 1.f)*rotate(mat4(1), radians(i*360.f / (float)div), vec3(0, 0, 1))
			+ vec4(r.x, 0.f, 0.f, 1.f);
		vec2 tx = vec2(0, (float)i / div);
		vec4 tg = vec4(0.f, -1.f, 0.f, 1.f)*rotate(mat4(1), radians(i*360.f / (float)div), vec3(0, 0, 1));
		vec3 n = cross(vec3(tg), vec3(0.f, 0.f, -1.f));
		vertex(vec3(p), vec3(n), vec3(tg), tx);
		frvtx.push_back({vec3(p), n, vec3(tg), tx});
	}

	for (int ring = 1; ring < ring_count + 1; ++ring)
	{
		mat4 rot = rotate(mat4(1), radians(ring*360.f / (float)div), vec3(0, 1, 0));
		for (int i = 0; i < stack_count + 1; ++i)
		{
			vec4 p = vec4(get<0>(frvtx[i]), 1.f);
			vec4 nr = vec4(get<1>(frvtx[i]), 0.f);
			vec4 tg = vec4(get<2>(frvtx[i]), 0.f);
			p = p*rot;
			nr = nr*rot;
			tg = tg*rot;

			vertex(vec3(p), vec3(nr),
				vec3(tg), vec2(2.f*ring / (float)div, get<3>(frvtx[i]).y));
		}
	}

	for (int ring = 0; ring < ring_count; ++ring)
	{
		for (int i = 0; i < stack_count; ++i)
		{
			index(ring*(div + 1) + i);
			index((ring + 1)*(div + 1) + i);
			index(ring*(div + 1) + i + 1);

			index(ring*(div + 1) + i + 1);
			index((ring + 1)*(div + 1) + i);
			index((ring + 1)*(div + 1) + i + 1);
		}
	}
}


inline vec3 conv(aiVector3D v) {
	return vec3(v.x, v.y, v.z);
}
inline mat4 conv(aiMatrix4x4 v) {
	return transpose(mat4(v.a1, v.a2, v.a3, v.a4,
		v.b1, v.b2, v.b3, v.b4,
		v.c1, v.c2, v.c3, v.c4,
		v.d1, v.d2, v.d3, v.d4));
}


/*struct vertex {
	vec3 pos;
	vec2 tex;

	static vk::VertexInputBindingDescription binding_desc() {
		return vk::VertexInputBindingDescription{ 0, sizeof(vertex) };
	}

	static array<vk::VertexInputAttributeDescription,2> attrib_desc() {
		return {
			vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, offsetof(vertex, pos)},
			vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32Sfloat, offsetof(vertex, tex)},
		};
	}
};
struct cam_uni_buf {
	mat4 model, view_proj;
};
struct test_app : public app {
	device dev;
	swap_chain swch;
	vk::UniquePipelineLayout layout;
	vk::UniqueRenderPass rnp;
	vk::UniquePipeline pp;
	shader_cache shc;

	vk::UniqueDescriptorPool descpool;
	vk::UniqueDescriptorSetLayout descset_layout;
	vk::UniqueDescriptorSet descset;

	unique_ptr<buffer> vbuf;
	unique_ptr<buffer> ibuf; uint32_t index_count;
	unique_ptr<buffer> ubuf;
	cam_uni_buf* ubuf_data;

	unique_ptr<image> tex;
	vk::UniqueImageView tex_view;
	vk::UniqueSampler samp;

	vector<vk::UniqueFramebuffer> framebuffers;
	vector<vk::UniqueCommandBuffer> cmd_bufs;

	test_app() :
		app("Vulkan Engine", vec2(1280, 960)),
		dev(this),
		swch(this, &dev), shc(&dev)
	{
		auto cb = move(dev.alloc_cmd_buffers()[0]);
		cb->begin(vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

#pragma region create vertex/index buffers
		vector<vertex> vertices;
		vector<uint32> indices;
		generate_torus(vec2(1.f, 0.5f), 32, [&vertices](auto p, auto, auto, auto tx) {
			vertices.push_back({ p,tx });
		}, [&indices](auto ix) { indices.push_back(ix); });
		index_count = indices.size();

		auto vstg_buf = buffer(&dev, sizeof(vertex)*vertices.size(), vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		auto istg_buf = buffer(&dev, sizeof(uint32)*indices.size(), vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		auto data = vstg_buf.map();
		memcpy(data, vertices.data(), sizeof(vertex)*vertices.size());
		vstg_buf.unmap();

		data = istg_buf.map();
		memcpy(data, indices.data(), sizeof(uint32)*indices.size());
		istg_buf.unmap();

		vbuf = unique_ptr<buffer>(new buffer(&dev, sizeof(vertex)*vertices.size(), 
			vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal));
		ibuf = unique_ptr<buffer>(new buffer(&dev, sizeof(uint32)*indices.size(), 
			vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal));

		cb->copyBuffer(vstg_buf, vbuf->operator vk::Buffer(), { vk::BufferCopy{0,0,sizeof(vertex)*vertices.size()} });
		cb->copyBuffer(istg_buf, ibuf->operator vk::Buffer(), { vk::BufferCopy{0,0,sizeof(uint32)*indices.size()} });
#pragma endregion

#pragma region create texture
		int w, h, ch;
		auto img = stbi_load("C:\\Users\\andre\\Source\\vkng\\vkng\\tex.png", &w, &h, &ch, STBI_rgb_alpha);
		vk::DeviceSize img_size = w*h*4;

		auto imgsb = buffer(&dev, img_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		data = imgsb.map();
		memcpy(data, img, img_size);
		imgsb.unmap();

		auto subresrange = vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0,1,0,1 };
		tex = make_unique<image>(&dev, vk::ImageType::e2D, vk::Extent3D(w, h, 1), vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal, 
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, &tex_view, vk::ImageViewType::e2D, subresrange);

		cb->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTopOfPipe, vk::DependencyFlags(), {}, {}, {
			vk::ImageMemoryBarrier{vk::AccessFlags(), vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, tex->operator vk::Image(), subresrange}
		});

		cb->copyBufferToImage(imgsb, tex->operator vk::Image(), vk::ImageLayout::eTransferDstOptimal, {
			vk::BufferImageCopy{0, 0, 0, vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1}, {0,0,0}, {(uint32_t)w,(uint32_t)h,1}}
		});
		
		cb->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTopOfPipe, vk::DependencyFlags(), {}, {}, {
			vk::ImageMemoryBarrier{vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, tex->operator vk::Image(), subresrange}
		});
#pragma endregion

		cb->end();
		dev.graphics_qu.submit({ vk::SubmitInfo{0,nullptr,nullptr,1,&cb.get()} }, nullptr);

		ubuf = make_unique<buffer>(&dev, sizeof(cam_uni_buf), vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, make_optional((void**)&ubuf_data));

		samp = dev.dev->createSamplerUnique(vk::SamplerCreateInfo{
			vk::SamplerCreateFlags(), vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eNearest, vk::SamplerAddressMode::eRepeat,
			vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, 0.f, VK_TRUE, 16.f
		});

		vector<vk::DescriptorPoolSize> pool_sizes = {
			{ vk::DescriptorType::eUniformBuffer, 1 },
			{ vk::DescriptorType::eCombinedImageSampler, 1 }
		};
		vk::DescriptorPoolCreateInfo pool_info{ vk::DescriptorPoolCreateFlags(), 1, pool_sizes.size(), pool_sizes.data() };
		descpool = dev.dev->createDescriptorPoolUnique(pool_info);

		descset_layout = dev.create_desc_set_layout({
			vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
			vk::DescriptorSetLayoutBinding{1, vk::DescriptorType::eSampler, 1, vk::ShaderStageFlagBits::eFragment}
		});

		vk::DescriptorSetLayout layouts[] = { descset_layout.get() };
		descset = move(dev.dev->allocateDescriptorSetsUnique(vk::DescriptorSetAllocateInfo{ descpool.get(), 1, layouts })[0]);

		vk::DescriptorBufferInfo bufifo{ ubuf->operator vk::Buffer(),  0, sizeof(cam_uni_buf) };
		dev.dev->updateDescriptorSets({
			{ descset.get(), 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &bufifo },
			{ descset.get(), 1, 0, 1, vk::DescriptorType::eCombinedImageSampler, &vk::DescriptorImageInfo{samp.get(), tex_view.get(), vk::ImageLayout::eShaderReadOnlyOptimal} }
		}, {});

		layout = dev.dev->createPipelineLayoutUnique(vk::PipelineLayoutCreateInfo { vk::PipelineLayoutCreateFlags(), 1, &descset_layout.get() });

		create_swapchain_resources();

		dev.graphics_qu.waitIdle();
	}

	void create_swapchain_resources() {
		vector<vk::AttachmentDescription> attachments = {
			{ vk::AttachmentDescriptionFlags(), //color
						swch.format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
						vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
						vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR },
			{ vk::AttachmentDescriptionFlags(), //depth
						vk::Format::eD32Sfloat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
						vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
						vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal }
		};
		vk::AttachmentReference col_ref{ 0, vk::ImageLayout::eColorAttachmentOptimal };
		vk::AttachmentReference dep_ref{ 1, vk::ImageLayout::eDepthStencilAttachmentOptimal };
		vector<vk::SubpassDependency> dpnd = { 
			{	VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::AccessFlagBits::eColorAttachmentRead, vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite, vk::DependencyFlags() },
		};

		vk::SubpassDescription subpass{ vk::SubpassDescriptionFlags(),
			vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &col_ref, nullptr, &dep_ref };
		vk::RenderPassCreateInfo rpcfo{ vk::RenderPassCreateFlags(), 
			attachments.size(), attachments.data(), 1, &subpass, dpnd.size(), dpnd.data() };
		rnp = dev.dev->createRenderPassUnique(rpcfo);
		


#pragma region Pipeline
		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = (VkShaderModule)shc.load_shader("shader.vert.spv");
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = (VkShaderModule)shc.load_shader("shader.frag.spv");
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
		viewport.width = (float)swch.extent.width;
		viewport.height = (float)swch.extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = swch.extent;

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

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
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
		pipelineInfo.layout = (VkPipelineLayout)layout.get();
		pipelineInfo.renderPass = (VkRenderPass)rnp.get();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		pp = dev.dev->createGraphicsPipelineUnique(VK_NULL_HANDLE, pipelineInfo);
#pragma endregion

		framebuffers = swch.create_framebuffers(rnp.get());

		cmd_bufs = dev.alloc_cmd_buffers(framebuffers.size());
		for (size_t i = 0; i < cmd_bufs.size(); ++i) {
			cmd_bufs[i]->begin(vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlagBits::eSimultaneousUse });

			vk::ClearValue cc[] = {
				vk::ClearColorValue{ array<float,4>{0.1f, 0.1f, 0.1f, 1.f} },
				vk::ClearDepthStencilValue{1.f, 0}
			};
			auto rbio = vk::RenderPassBeginInfo{ rnp.get(), framebuffers[i].get(),
				vk::Rect2D(vk::Offset2D(), swch.extent), 2, cc };
			cmd_bufs[i]->beginRenderPass(rbio, vk::SubpassContents::eInline);

			cmd_bufs[i]->bindPipeline(vk::PipelineBindPoint::eGraphics, pp.get());
			cmd_bufs[i]->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout.get(), 0, { descset.get() }, { });

			vk::Buffer bufs[] = { vk::Buffer(vbuf->buf) };
			vk::DeviceSize offsets[] = { 0 };
			cmd_bufs[i]->bindVertexBuffers(0, 1, bufs, offsets);
			cmd_bufs[i]->bindIndexBuffer(ibuf->operator vk::Buffer(), 0, vk::IndexType::eUint32);

			cmd_bufs[i]->drawIndexed(index_count, 1, 0, 0, 0);

			cmd_bufs[i]->endRenderPass();
			cmd_bufs[i]->end();
		}
	}

	void update(float t, float dt) override {
		auto proj = perspective(pi<float>() / 2.f, swch.extent.width / (float)swch.extent.height, 0.1f, 10.f);
		proj[1][1] *= -1; //hack to flip Y axis
		ubuf_data->view_proj = proj
			* lookAt(vec3(0.f,1.f,2.f), vec3(0.f, 0.f, 0.f), vec3(0.f, 0.f, 1.f));
		ubuf_data->model = rotate(mat4(1), t*2.f, vec3(1.0f, 0.4f, .0f));
	}
	void recreate_swapchain() {
		for (auto& fb : framebuffers) fb.reset();
		for (auto& cb : cmd_bufs) cb.reset();
		pp.reset();
		//layout.reset();
		//rnp.reset();
		swch.recreate(this);
		create_swapchain_resources();
	}
	void resize() override {
		recreate_swapchain();
	}
	void render(float t, float dt) override {
		auto img_idx = swch.aquire_next();
		if (!img_idx.ok() && img_idx.err() == vk::Result::eErrorOutOfDateKHR || img_idx.err() == vk::Result::eSuboptimalKHR) {
			recreate_swapchain();
			return;
		}
		vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		vk::SubmitInfo sfo{ 1, &swch.image_ava_sp.get(), wait_stages, 1, &cmd_bufs[img_idx].get(), 
								1, &swch.render_fin_sp.get() };
		dev.graphics_qu.submit(sfo, VK_NULL_HANDLE);
		swch.present(img_idx);
	}
};*/

struct test_app : public app {
	device dev; swap_chain swp; shader_cache shc;
	unique_ptr<renderer::renderer> rndr;
	unique_ptr<image> tex;
	vk::UniqueImageView tex_view;
	perspective_camera cam;
	fps_camera_controller ctrl;

	test_app() :
		app("Vulkan Engine", vec2(1280, 960)),
		dev(this), swp(this, &dev),
		shc(&dev),
		cam(vec2(1280, 960), vec3(0.f, 5.f, 5.f), vec3(0.f), vec3(0.f, 1.f, 0.f), 
			pi<float>()/3.f, 1.f, 3500.f),
		ctrl(cam, vec3(100.f))
	{
		vector<renderer::vertex> vertices;
		vector<uint32> indices;
		generate_torus(vec2(1.f, 0.5f), 32, [&vertices](auto p, auto n, auto tg, auto tx) {
			vertices.push_back({ p,n,tg,tx });
		}, [&indices](auto ix) { indices.push_back(ix); });
		
		int w, h, ch;
		auto img = stbi_load("C:\\Users\\andre\\Source\\vkng\\vkng\\tex.png", &w, &h, &ch, STBI_rgb_alpha);
		vk::DeviceSize img_size = w*h * 4;

		auto imgsb = buffer(&dev, img_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		auto data = imgsb.map();
		memcpy(data, img, img_size);
		imgsb.unmap();

		auto subresrange = vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0,1,0,1 };
		tex = make_unique<image>(&dev, vk::ImageType::e2D, vk::Extent3D(w, h, 1), vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal, 
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal,
			&tex_view, vk::ImageViewType::e2D, subresrange);

		auto cb = move(dev.alloc_cmd_buffers()[0]);
		cb->begin(vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
		cb->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTopOfPipe, vk::DependencyFlags(), {}, {}, {
			vk::ImageMemoryBarrier{vk::AccessFlags(), vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, tex->operator vk::Image(), subresrange}
		});

		cb->copyBufferToImage(imgsb, tex->operator vk::Image(), vk::ImageLayout::eTransferDstOptimal, {
			vk::BufferImageCopy{0, 0, 0, vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1}, {0,0,0}, {(uint32_t)w,(uint32_t)h,1}}
		});
		
		cb->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTopOfPipe, vk::DependencyFlags(), {}, {}, {
			vk::ImageMemoryBarrier{vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, tex->operator vk::Image(), subresrange}
		});
		cb->end();
		dev.graphics_qu.submit({ vk::SubmitInfo{0, nullptr, nullptr, 1, &cb.get()} }, nullptr); // start actually copying stuff while we create everything else
		
		vector<renderer::object_desc> objects;

		Assimp::Importer imp;
		auto scene = imp.ReadFile("C:\\Users\\andre\\Downloads\\3DModels\\sponza\\sponza.obj", aiProcessPreset_TargetRealtime_Fast);

		stack<aiNode*> nodes;
		nodes.push(scene->mRootNode);
		while (!nodes.empty()) {
			auto node = nodes.top(); nodes.pop();
			for (size_t i = 0; i < node->mNumChildren; ++i) nodes.push(node->mChildren[i]);
			for (size_t mix = 0; mix < node->mNumMeshes; ++mix) {
				auto mesh = scene->mMeshes[node->mMeshes[mix]];
				if (!(mesh->HasPositions() && mesh->HasNormals() && mesh->HasTextureCoords(0) && mesh->HasTangentsAndBitangents())) {
					cout << "mesh " << mesh->mName.C_Str() << " isn't complete" << endl;
					continue;
				}
				vector<renderer::vertex> vertices; vector<uint32> indices;
				for (size_t vi = 0; vi < mesh->mNumVertices; ++vi) {
					vertices.push_back({
						conv(mesh->mVertices[vi]),
						conv(mesh->mNormals[vi]),
						conv(mesh->mTangents[vi]),
						conv(mesh->mTextureCoords[0][vi])
					});
				}
				for (size_t fi = 0; fi < mesh->mNumFaces; ++fi) {
					for (size_t ii = 0; ii < mesh->mFaces[fi].mNumIndices; ++ii)
						indices.push_back(mesh->mFaces[fi].mIndices[ii]);
				}
				objects.push_back({ vertices, indices, conv(node->mTransformation), tex_view.get() });
			}
		}
		
		rndr = make_unique<renderer::renderer>(&dev, &swp, &shc, &cam, objects);

		input_handlers.push_back(&ctrl);
	}

	void update(float t, float dt) override {
		/*for (int i = 0; i < 5; ++i) {
			*rndr->objects[i].transform = translate(rotate(mat4(1), t*2.f, vec3(0.2f, 0.6f, 0.4f)),
				vec3( i*3.f - 6.f, 0.f, 0.f));
		}*/
		ctrl.update(t, dt);
	}
	void resize() override {
		rndr->reset();
		swp.recreate(this);
		rndr->recreate(&swp);
	}
	void render(float t, float dt) override {
		auto img_idx = swp.aquire_next();
		if (!img_idx.ok() && img_idx.err() == vk::Result::eErrorOutOfDateKHR || img_idx.err() == vk::Result::eSuboptimalKHR) {
			resize();
			return;
		}

		auto cb = rndr->render(img_idx);

		vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		vk::SubmitInfo sfo{ 1, &swp.image_ava_sp.get(), wait_stages, 1, &cb, 
								1, &swp.render_fin_sp.get() };
		dev.graphics_qu.submit(sfo, VK_NULL_HANDLE);
		swp.present(img_idx);
	
	}
};

int main() {
	test_app _;
	_.run();
	return 0;
}