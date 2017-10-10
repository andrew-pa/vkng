#include "camera.h"

namespace vkng {
	perspective_camera::perspective_camera(vec2 ss, vec3 p, vec3 t, vec3 u, float fov, float nz, float fz)
		: camera(p, t, u), _fov(fov), _nz(nz), _fz(fz)
	{
		update_view();
		update_proj(ss);
	}


	void perspective_camera::update_proj(vec2 size)
	{
		_proj = perspectiveFov(_fov, size.x, size.y, _nz, _fz);
		_proj[1][1] *= -1.f;
		_ar = size.x / size.y;
	}

	void camera::update_view()
	{
		_look = normalize(_look);
		_up = normalize(_up);
		_right = cross(_look, _up);
		_up = cross(_right, _look);

		float px = -dot(_right, _pos);
		float py = -dot(_up, _pos);
		float pz = dot(_look, _pos);

		_view = mat4(1);
		_view[0][0] = _right.x;
		_view[1][0] = _right.y;
		_view[2][0] = _right.z;
		_view[0][1] = _up.x;
		_view[1][1] = _up.y;
		_view[2][1] = _up.z;
		_view[0][2] = -_look.x;
		_view[1][2] = -_look.y;
		_view[2][2] = -_look.z;
		_view[3][0] = px;
		_view[3][1] = py;
		_view[3][2] = pz;
	}

	void camera::transform(const mat4& t)
	{
		_look = vec3(vec4(_look, 0)*t);
		_up = vec3(vec4(_up, 0)*t);
		_right = vec3(vec4(_right, 0)*t);
	}

	void fps_camera_controller::key_handler(app* _app, uint key, input_action action, input_mod mods) {
		if (key == GLFW_KEY_F1 && action == input_action::release) {
			mouse_disabled = !mouse_disabled;
			if (glfwGetInputMode(_app->wnd, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
				glfwSetInputMode(_app->wnd, GLFW_CURSOR, normal_cursor_mode);
		}
		if (key == GLFW_KEY_F2 && action == input_action::release) {
			cam.look_at(vec3(0.f, 2.f, 5.f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
		}
		if (action == key_action::press)
		{
			if (key == GLFW_KEY_W)
				cam_pos_v.x = 1;
			else if (key == GLFW_KEY_S)
				cam_pos_v.x = -1;
			else if (key == GLFW_KEY_A)
				cam_pos_v.y = -1;
			else if (key == GLFW_KEY_D)
				cam_pos_v.y = 1;
			else if (key == GLFW_KEY_Q)
				cam_pos_v.z = 1;
			else if (key == GLFW_KEY_E)
				cam_pos_v.z = -1;
			else if (key == GLFW_KEY_UP)
				cam_rot_v.y = -1;
			else if (key == GLFW_KEY_DOWN)
				cam_rot_v.y = -1;
			else if (key == GLFW_KEY_LEFT)
				cam_rot_v.x = -1;
			else if (key == GLFW_KEY_RIGHT)
				cam_rot_v.x = -1;
		}
		else if (action == key_action::release)
		{
			if (key == GLFW_KEY_W || key == GLFW_KEY_S)
				cam_pos_v.x = 0.f;
			else if (key == GLFW_KEY_A || key == GLFW_KEY_D)
				cam_pos_v.y = 0.f;
			else if (key == GLFW_KEY_Q || key == GLFW_KEY_E)
				cam_pos_v.z = 0.f;
			else if (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN)
				cam_rot_v.y = 0.f;
			else if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT)
				cam_rot_v.x = 0.f;
		}

	}
	void fps_camera_controller::mouse_position_handler(app * _app, vec2 _p) {
		if (!mouse_disabled) {
			if (glfwGetInputMode(_app->wnd, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
				glfwSetInputMode(_app->wnd, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			auto smid = vec2(_app->size());
			vec2 np = (_p / smid)*2.f - vec2(1.f);
			cam_rot_v = np*60.f;
			glfwSetCursorPos(_app->wnd, smid.x*0.5f, smid.y*0.5f);
		}
	}
	void fps_camera_controller::update(float t, float dt) {
		cam.fwd(cam_pos_v.x*dt*linear_speed.x);
		cam.straft(cam_pos_v.y*dt*linear_speed.y);
		cam.move_up(cam_pos_v.z*dt*linear_speed.z);
		cam.transform(rotate(mat4(1), cam_rot_v.x*dt*rotational_speed.x, vec3(0.f, 1.f, 0.f)));
		cam.pitch(cam_rot_v.y*dt*rotational_speed.y);
		cam_rot_v = vec2(0.f);
		cam.update_view();
	}
}