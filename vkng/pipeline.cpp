#include "pipeline.h"

namespace vkng {
	pipeline::pipeline(device* d, vk::PipelineLayout layout, vk::RenderPass renderpass, 
		uint32_t subpass, options b, vk::Pipeline basepipe, int32_t baseindex) {

		vk::PipelineViewportStateCreateInfo vp;
		vp.pScissors = &b.scissor; vp.pViewports = &b.viewport_;
		vp.viewportCount = vp.scissorCount = 1;
		vk::GraphicsPipelineCreateInfo ifo {
			vk::PipelineCreateFlags(), b.shaders.size(), b.shaders.data(),
			&b.vi, &b.ia, nullptr, &vp, &b.rs, &b.multisample_ifo, nullptr, &b.blend_,
			b.dynamic_states_.size() > 0 ? 
				&vk::PipelineDynamicStateCreateInfo(vk::PipelineDynamicStateCreateFlags(), 
					b.dynamic_states_.size(), b.dynamic_states_.data()) 
				: nullptr,
			layout, renderpass, subpass, basepipe, baseindex 
		};
		
		pp = d->dev->createGraphicsPipelineUnique(VK_NULL_HANDLE, ifo);
	}
	pipeline::~pipeline() {
	}
}