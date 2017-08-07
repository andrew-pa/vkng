#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL 
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <string>
#include <sstream>
#include <vector>
#include <stack>
#include <queue>
#include <map>
#include <iostream>
#include <fstream>
#include <functional>
#include <exception>
#include <algorithm>
#include <chrono>
#include <thread>
#include <memory>
#include <optional>
using namespace std;

#include <vulkan\vulkan.hpp>

template<typename T, typename E>
class result {
	uint8_t type;
	union { T t; E e; };
public:
	result(T t) : t(t), type(0) {}
	result(E e) : e(e), type(1) {}

	operator T() {
		if (type) throw runtime_error("tried to unwrap error result");
		return t;
	}

	inline bool ok() { return !type; }
	inline E err() { return e; }

	template<typename U>
	inline result<U,E> map(function<U(T)> f) {
		if (type) return result<U, E>(e);
		else return result<U, E>(f(t));
	}

	template<typename G>
	inline result<T, G> map_err(function<G(E)> f) {
		if (!type) return result<T, G>(t);
		else return result<T, G>(f(e));
	}
};