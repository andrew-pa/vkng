#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
	mat4 view_proj;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 texCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
	texCoord = inPosition;
	gl_Position = (pc.view_proj*vec4(inPosition, 1.0));
}
