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
	test_app() : 
		app("Vulkan Engine", vec2(1280, 960)),
		dev(this),
		swch(this, &dev)
	{
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