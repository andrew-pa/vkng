#pragma once
#include "cmmn.h"
#include "app.h"
namespace vkng {
	struct camera
	{
		mat4 _view, _proj;
		vec3 _pos, _look, _up, _right;
		camera() {}
		camera(vec3 p, vec3 t, vec3 u = vec3(0.f, 1.f, 0.f))
			: _pos(p), _look(t - p), _up(u)
		{
			_right = cross(_up, _look);
			update_view();
		}

		virtual void update_proj(vec2 size) = 0;
		void update_view();

		inline void look_at(vec3 p, vec3 t, vec3 u)
		{
			_pos = p; _look = t - _pos; _up = u;
		}

		inline void target(vec3 p) { _look = p - _pos; }


		inline void fwd(float d) { _pos += d*_look; }
		inline void straft(float d) { _pos += d*_right; }
		inline void move_up(float d) { _pos += d*_up; }

		void transform(const mat4& t);

		inline void yaw(float a) { transform(rotate(mat4(1), a, _up)); }
		inline void pitch(float a) { transform(rotate(mat4(1), a, _right)); }
		inline void roll(float a) { transform(rotate(mat4(1), a, _look)); }

		mat3 basis() const
		{
			return mat3(_look, _up, _right);
		}
	};

	struct perspective_camera : public camera {
		float _fov, _nz, _fz, _ar;
		perspective_camera(vec2 ss, vec3 p, vec3 t, vec3 u = vec3(0.f, 1.f, 0.f), float fov = radians(45.f),
			float nz = 0.5f, float fz = 500.f);
		void update_proj(vec2 size) override;
	};



	struct fps_camera_controller : public input_handler {
	protected:
		vec3 cam_pos_v; vec2 cam_rot_v; vec2 tot_cam_rot;
	public:
		perspective_camera& cam;
		vec3 linear_speed;
		vec2 rotational_speed;
		bool mouse_disabled;
		uint normal_cursor_mode;
		fps_camera_controller(perspective_camera& c, vec3 lin_speed = vec3(7.f, 10.f, 5.f), vec2 rot_speed = vec2(1.f))
			: cam(c), linear_speed(lin_speed), rotational_speed(rot_speed), mouse_disabled(false), normal_cursor_mode(GLFW_CURSOR_NORMAL) {
		}

		virtual void key_handler(app* _app, uint key, input_action action, input_mod mods);
		virtual void mouse_position_handler(app* _app, vec2 _p);
		void update(float t, float dt);
	};
}
