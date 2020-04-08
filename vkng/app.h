#pragma once
#include "cmmn.h"
#include "timer.h"

namespace vkng
{
	enum class input_action
	{
		press = GLFW_PRESS,
		release = GLFW_RELEASE,
		repeat = GLFW_REPEAT,
	};
	typedef input_action key_action;

	enum class input_mod
	{
		alt = GLFW_MOD_ALT,
		ctrl = GLFW_MOD_CONTROL,
		shift = GLFW_MOD_SHIFT,
		super = GLFW_MOD_SUPER,
	};
	typedef input_mod key_mod;

	enum class mouse_button {
		left = GLFW_MOUSE_BUTTON_LEFT,
		right = GLFW_MOUSE_BUTTON_RIGHT,
		middle = GLFW_MOUSE_BUTTON_MIDDLE,
	};

	class app;

	struct input_handler {
		virtual void key_handler(app* _app, uint keycode, input_action action, input_mod mods){}
		virtual void char_handler(app* _app, uint codepoint, input_mod mods){}
		virtual void mouse_position_handler(app* _app, vec2 p){}
		virtual void mouse_enterleave_handler(app* _app, bool entered){}
		virtual void mouse_button_handler(app* _app, mouse_button button, input_action action, input_mod mods){}
	};

	class app
	{
		bool _did_resize;
	public:
		timer tm;
		GLFWwindow* wnd;
		vk::Instance instance;
		vk::SurfaceKHR surface;
#ifdef DEBUG
		VkDebugReportCallbackEXT report_callback;
#endif

		app(const std::string& title, vec2 winsize);
		~app();

		void run(bool print_debug_fps = true);

		virtual void render(float t, float dt) = 0;
		virtual void update(float t, float dt) = 0;
		virtual void make_resolution_dependent_resources(vec2 size) {}
		virtual void resize() { }
		virtual void key_down(uint keycode, key_action action, key_mod mods) {}

		inline ivec2 size() {
			ivec2 wh;
			glfwGetWindowSize(wnd, &wh.x, &wh.y);
			return wh;
		}

		std::vector<input_handler*> input_handlers; //~app does not delete these!
	};
}
