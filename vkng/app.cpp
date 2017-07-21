#include "app.h"
#include "timer.h"

namespace vkng
{
	app::app(const string& title, vec2 winsize)
	{
		if (!glfwInit()) throw runtime_error("GLFW init failed!");
		glfwSetErrorCallback([](int ec, const char* em) {
			char s[64]; sprintf_s(s, 64, "GLFW error: %s, (error code: %08X)", em, ec);
			throw runtime_error(s);
		});

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		wnd = glfwCreateWindow((int)floor(winsize.x), (int)floor(winsize.y), title.c_str(), nullptr, nullptr);
		if(!wnd)
		{
			glfwTerminate();
			throw runtime_error("GLFW window creation failed");
		}
	
		glfwSetWindowUserPointer(wnd, this);

		glfwSetWindowSizeCallback(wnd, [](GLFWwindow* wnd, int w, int h)
		{
			auto t = (app*)glfwGetWindowUserPointer(wnd);
			t->resize();
			t->make_resolution_dependent_resources(vec2(w,h));
		});

		glfwSetKeyCallback(wnd, [](GLFWwindow* wnd, int key, int scancode, int action, int mods) 
		{
			auto t = (app*)glfwGetWindowUserPointer(wnd);
			t->key_down(key, (key_action)action, (key_mod)mods);
			for(auto& ih : t->input_handlers) {
				ih->key_handler(t, key, (input_action)action, (input_mod)mods);
			}
		});

		glfwSetCharModsCallback(wnd, [](GLFWwindow* wnd, uint cp, int mods) {
			auto t = (app*)glfwGetWindowUserPointer(wnd);
			for (auto& ih : t->input_handlers) {
				ih->char_handler(t, cp, (input_mod)mods);
			}
		});
		
		glfwSetCursorPosCallback(wnd, [](GLFWwindow* wnd, double x, double y) {
			auto t = (app*)glfwGetWindowUserPointer(wnd);
			for (auto& ih : t->input_handlers) {
				ih->mouse_position_handler(t, vec2(x, y));
			}
		});

		glfwSetCursorEnterCallback(wnd, [](GLFWwindow* wnd, int entered) {
			auto t = (app*)glfwGetWindowUserPointer(wnd);
			for (auto& ih : t->input_handlers) {
				ih->mouse_enterleave_handler(t, entered>0);
			}
		});

		glfwSetMouseButtonCallback(wnd, [](GLFWwindow* wnd, int button, int action, int mods) {
			auto t = (app*)glfwGetWindowUserPointer(wnd);
			for (auto& ih : t->input_handlers) {
				ih->mouse_button_handler(t, (mouse_button)button, (input_action)action, (input_mod)mods);
			}
		});
		vk::InstanceCreateInfo icfo;
		icfo.pApplicationInfo = &vk::ApplicationInfo {
			title.c_str(), VK_MAKE_VERSION(0,1,0),
			"vkng", VK_MAKE_VERSION(0,1,0),
			VK_API_VERSION_1_0
		};
		uint glfw_ext_cnt = 0;
		const char** glfw_ext;
		glfw_ext = glfwGetRequiredInstanceExtensions(&glfw_ext_cnt);
		icfo.enabledExtensionCount = glfw_ext_cnt;
		icfo.ppEnabledExtensionNames = glfw_ext;
		icfo.enabledLayerCount = 0;
		instance = vk::createInstance(icfo);

		VkSurfaceKHR sf;
		auto res = glfwCreateWindowSurface((VkInstance)instance,
			wnd, nullptr, &sf);
		surface = vk::SurfaceKHR(sf);
	}

	void app::run(bool pdfps)
	{
		const float target_delta_time = 1.f / 120.f;

		uint fc = 0;
		float ft = 0.f;
		tm.reset();
		while(!glfwWindowShouldClose(wnd))
		{
			tm.update();
			update(tm.time(), tm.delta_time());
			render(tm.time(), tm.delta_time());
			fc++;
			ft += tm.delta_time();
			glfwPollEvents();
			if(tm.delta_time() < target_delta_time)
			{
				auto missing_delta = target_delta_time - tm.delta_time();
				if(missing_delta > 0.f)
					this_thread::sleep_for(chrono::milliseconds((long)ceil(missing_delta)));
			}
		}
	}

	app::~app()
	{
		vkDestroySurfaceKHR((VkInstance)instance, (VkSurfaceKHR)surface, nullptr);
		instance.destroy();
		glfwDestroyWindow(wnd);
		glfwTerminate();
	}
}
