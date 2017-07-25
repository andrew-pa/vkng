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
	unique_ptr<pipeline> pp;
	vk::UniquePipelineLayout layout;
	vk::UniqueRenderPass rnp;
	shader_cashe shc;

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
		vk::SubpassDescription subpass{ vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &col_ref };
		vk::RenderPassCreateInfo rpcfo{ vk::RenderPassCreateFlags(), 1, &color_attachment, 1, &subpass };
		rnp = dev.dev->createRenderPassUnique(rpcfo);
		pp = make_unique<pipeline>(&dev, layout.get(), rnp.get(), 0,
			pipeline::options()
				.input_assembly(vk::PrimitiveTopology::eTriangleList)
				.viewport(vec2(swch.extent.width, swch.extent.height))
				.rasterizer()
				.blend(vk::PipelineColorBlendStateCreateInfo{}, { vk::PipelineColorBlendAttachmentState {} })
				.vertex_shader(shc.load_shader("shader.vert.spv"))
				.fragment_shader(shc.load_shader("shader.frag.spv"))
		);
	}

	void update(float t, float dt) override {

	}
	void resize() override {

	}
	void render(float t, float dt) override {

	}
};

int main() {
	test_app _;
	_.run();
}