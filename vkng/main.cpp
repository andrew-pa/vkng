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


	Roadmap:
		[0] Cubemap Skys
		[0] Normal Maps
		[1] Directional lights, composited into buffer
		[1] proper tonemapping/gamma correction + vignette/noise
		[2] Physically Based Shading
		[3] Shadows
		[3?] Scripting
		[4] Voxels (Octree)
		[4] Point Lights
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

struct test_app : public app {
	device dev; swap_chain swp; shader_cache shc;
	unique_ptr<renderer::renderer> rndr;
	unique_ptr<image> tex;
	vk::UniqueImageView tex_view;

	vector<unique_ptr<image>> diffuse_textures;
	map<size_t, vk::UniqueImageView> diffuse_texture_views;

	unique_ptr<image> sky;
	vk::UniqueImageView sky_view;

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
		/*vector<renderer::vertex> vertices;
		vector<uint32> indices;
		generate_torus(vec2(1.f, 0.5f), 32, [&vertices](auto p, auto n, auto tg, auto tx) {
			vertices.push_back({ p,n,tg,tx });
		}, [&indices](auto ix) { indices.push_back(ix); });*/
	
		// load squid texture
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

		// load sky cubemap
		unique_ptr<buffer> sky_staging;
		{
			const string cubemap_path = "C:\\Users\\andre\\Source\\vkng\\vkng\\MountainPath\\";
			vector<void*> img_data;
			for (const auto& path : { "posx.jpg", "negx.jpg", "posy.jpg", "negy.jpg", "posz.jpg", "negz.jpg" }) {
				img_data.push_back(stbi_load((cubemap_path + path).c_str(), &w, &h, &ch, STBI_rgb_alpha));
			}
			img_size = w*h * 4;
			sky_staging = make_unique<buffer>(&dev, img_size * img_data.size(), vk::BufferUsageFlagBits::eTransferSrc, 
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			auto data = (char*)sky_staging->map();
			vector<vk::BufferImageCopy> copy_descs;
			for (size_t i = 0; i < img_data.size(); ++i) {
				memcpy(data + i*img_size, img_data[i], img_size);
				copy_descs.push_back(vk::BufferImageCopy{ i*img_size, 0, 0, vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, i, 1},
							{0,0,0}, {(uint32_t)w,(uint32_t)h,1} });
			}
			sky_staging->unmap();
			subresrange = vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6 };
			sky = make_unique<image>(&dev, vk::ImageCreateFlagBits::eCubeCompatible, vk::ImageType::e2D, vk::Extent3D(w, h, 1), vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, 1, 6,
				&sky_view, vk::ImageViewType::eCube, subresrange);

			cb->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTopOfPipe, vk::DependencyFlags(), {}, {}, {
				vk::ImageMemoryBarrier{vk::AccessFlags(), vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
					VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, sky->operator vk::Image(), subresrange}
			});

			cb->copyBufferToImage(sky_staging->operator vk::Buffer(), sky->operator vk::Image(), vk::ImageLayout::eTransferDstOptimal, copy_descs);

			cb->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTopOfPipe, vk::DependencyFlags(), {}, {}, {
				vk::ImageMemoryBarrier{vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
					VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, sky->operator vk::Image(), subresrange}
			});
		}
		
		// load scene
		vector<renderer::object_desc> objects;

		Assimp::Importer imp;
		auto mesh_path = //string("C:\\Users\\andre\\Source\\vkng\\vkng\\"); 
						string("C:\\Users\\andre\\Downloads\\3DModels\\sponza\\");
		auto scene = imp.ReadFile(mesh_path + "sponza.obj", aiProcessPreset_TargetRealtime_Fast);
		cout << "assimp finished" << endl;

		stbi_set_flip_vertically_on_load(true);
		vector<unique_ptr<buffer>> upload_buffers;
		for (size_t i = 0; i < scene->mNumMaterials; ++i) {
			auto mat = scene->mMaterials[i];
			aiString path;
			if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == aiReturn_SUCCESS) {
				int w, h, ch;
				auto img = stbi_load((mesh_path + path.C_Str()).c_str(), &w, &h, &ch, STBI_rgb_alpha);
				if (img == nullptr) {
					cout << "couldn't load image " << path.C_Str() << endl;
					continue;
				}
				vk::DeviceSize img_size = w*h * 4;

				// this piece of code could be much more effecient:
				// all images could be loaded, then a buffer allocated for all of them, each one copied in
				// the pipeline barriers and copy ops could then be collected into vectors,
				// and then all submitted together under unified commands in the command buffer (reduction from N commands -> 3)
				auto imgsb = make_unique<buffer>(&dev, img_size, vk::BufferUsageFlagBits::eTransferSrc,
					vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
				auto data = imgsb->map();
				memcpy(data, img, img_size);
				imgsb->unmap();

				auto subresrange = vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0,1,0,1 };
				vk::UniqueImageView view;
				diffuse_textures.push_back(make_unique<image>(&dev, vk::ImageType::e2D, vk::Extent3D(w, h, 1), vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal,
					vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal,
					&view, vk::ImageViewType::e2D, subresrange));
				auto& tx = diffuse_textures[diffuse_textures.size() - 1];

				cb->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTopOfPipe, vk::DependencyFlags(), {}, {}, {
					vk::ImageMemoryBarrier{vk::AccessFlags(), vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
						VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, tx->operator vk::Image(), subresrange}
				});

				cb->copyBufferToImage(imgsb->operator vk::Buffer(), tx->operator vk::Image(), vk::ImageLayout::eTransferDstOptimal, {
					vk::BufferImageCopy{0, 0, 0, vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1}, {0,0,0}, {(uint32_t)w,(uint32_t)h,1}}
				});

				cb->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTopOfPipe, vk::DependencyFlags(), {}, {}, {
					vk::ImageMemoryBarrier{vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
						VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, tx->operator vk::Image(), subresrange}
				});

				upload_buffers.push_back(move(imgsb));
				diffuse_texture_views[i] = move(view);
			}
		}
		cb->end();
		dev.graphics_qu.submit({ vk::SubmitInfo{0, nullptr, nullptr, 1, &cb.get()} }, nullptr); // start actually copying stuff while we create everything else
		cout << "textures loaded" << endl;

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
				vk::ImageView tv = tex_view.get();
				auto pdtv = diffuse_texture_views.find(mesh->mMaterialIndex);
				if (pdtv != diffuse_texture_views.end()) tv = pdtv->second.get();
				objects.push_back({ vertices, indices, conv(node->mTransformation), tv });
			}
		}
		cout << "meshes loaded" << endl;
		
		rndr = make_unique<renderer::renderer>(&dev, &swp, &shc, &cam, objects, sky_view.get());

		input_handlers.push_back(&ctrl);
		cout << "initialization finished!" << endl;
	}

	void update(float t, float dt) override {
	/*	for (int i = 0; i < 1; ++i) {
			*rndr->objects[i].transform = rotate(mat4(1), t*2.f, vec3(0.2f, 0.6f, 0.4f));
		} */
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