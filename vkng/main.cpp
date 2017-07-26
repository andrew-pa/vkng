#include "cmmn.h"
#include "app.h"
#include "device.h"
#include "swap_chain.h"
#include "pipeline.h"

namespace vkng {
}

using namespace vkng;

struct test_app : public app {
	device dev;
	swap_chain swch;
	//unique_ptr<pipeline> pp;
	vk::UniquePipelineLayout layout;
	vk::UniqueRenderPass rnp;
	vk::UniquePipeline pp;
	shader_cashe shc;

	vector<vk::UniqueFramebuffer> framebuffers;
	vector<vk::UniqueCommandBuffer> cmd_bufs;

	test_app() :
		app("Vulkan Engine", vec2(1280, 960)),
		dev(this),
		swch(this, &dev), shc(&dev)
	{
		vk::PipelineLayoutCreateInfo lyf;
		layout = dev.dev->createPipelineLayoutUnique(lyf);

		vk::AttachmentDescription color_attachment{ vk::AttachmentDescriptionFlags(),
			swch.format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR };
		vk::AttachmentReference col_ref{ 0, vk::ImageLayout::eColorAttachmentOptimal };
		vk::SubpassDependency dpnd{ VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentRead, vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
		vk::DependencyFlags() };

		vk::SubpassDescription subpass{ vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &col_ref };
		vk::RenderPassCreateInfo rpcfo{ vk::RenderPassCreateFlags(), 1, &color_attachment, 1, &subpass, 1, &dpnd };
		rnp = dev.dev->createRenderPassUnique(rpcfo);





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
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;

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
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

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
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = (VkPipelineLayout)layout.get();
		pipelineInfo.renderPass = (VkRenderPass)rnp.get();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		pp = dev.dev->createGraphicsPipelineUnique(VK_NULL_HANDLE, pipelineInfo);

		framebuffers.resize(swch.image_views.size());
		for (size_t i = 0; i < swch.image_views.size(); ++i) {
			vk::ImageView att[] = {
				swch.image_views[i].get()
			};
			dev.dev->createFramebufferUnique(vk::FramebufferCreateInfo { vk::FramebufferCreateFlags(), rnp.get(), 1, att,
				swch.extent.width, swch.extent.height, 1 });
		}

		cmd_bufs = dev.alloc_cmd_buffers(framebuffers.size());
		for (size_t i = 0; i < cmd_bufs.size(); ++i) {
			cmd_bufs[i]->begin(vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlagBits::eSimultaneousUse });

			vk::ClearValue cc = vk::ClearColorValue(array<float,4>{0.8f, 0.6f, 0.f, 1.f});
			cmd_bufs[i]->beginRenderPass(vk::RenderPassBeginInfo{ rnp.get(), framebuffers[i].get(), 
				vk::Rect2D(vk::Offset2D(), swch.extent), 1, &cc}, vk::SubpassContents::eInline);

			cmd_bufs[i]->bindPipeline(vk::PipelineBindPoint::eGraphics, pp.get());
			cmd_bufs[i]->draw(3, 1, 0, 0);

			cmd_bufs[i]->endRenderPass();

			cmd_bufs[i]->end();
		}

		/*pp = make_unique<pipeline>(&dev, layout.get(), rnp.get(), 0,
			pipeline::options()
				.input_assembly(vk::PrimitiveTopology::eTriangleList)
				.viewport(vec2(swch.extent.width, swch.extent.height))
				.rasterizer()
				.blend(vk::PipelineColorBlendStateCreateInfo{}, { vk::PipelineColorBlendAttachmentState {} })
				.vertex_shader(shc.load_shader("shader.vert.spv"))
				.fragment_shader(shc.load_shader("shader.frag.spv"))
		);*/
	}

	void update(float t, float dt) override {

	}
	void resize() override {

	}
	void render(float t, float dt) override {
		auto img_idx = swch.aquire_next(&dev);
		vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		vk::SubmitInfo sfo{ 1, &swch.image_ava_sp.get(), wait_stages, 1, &cmd_bufs[img_idx].get(), 
								1, &swch.render_fin_sp.get() };
		dev.graphics_qu.submit(sfo, VK_NULL_HANDLE);
	}
};

int main() {
	test_app _;
	_.run();
}