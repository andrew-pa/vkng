#pragma once
#include "cmmn.h"

namespace vkng
{
	class timer
	{
		double last_time;
		double curr_time;
		double _ctime;
		double _deltat;
	public:
		timer()
		{
			last_time = curr_time = glfwGetTime();
			_deltat = 0;
			_ctime = 0;
		}

		void reset()
		{
			last_time = curr_time = glfwGetTime();
			_ctime = 0;
			_deltat = 0;
		}

		void update()
		{
			curr_time = glfwGetTime();

			_deltat = curr_time - last_time;
			_ctime += _deltat;

			last_time = curr_time;
		}

		inline float time() const { return (float)_ctime; }
		inline float delta_time() const { return (float)_deltat; }
	};
}