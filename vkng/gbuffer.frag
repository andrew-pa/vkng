#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec3 worldCoord;
layout(location = 2) in vec3 worldNormal;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outColor;
layout(location = 3) out vec4 outStuff;

layout(binding = 1) uniform sampler2D tex;

void main() {
	outPosition = vec4(worldCoord,0.f);
	outNormal = vec4(worldNormal,0.f);
    outColor = texture(tex, texCoord);
	outStuff = gl_FragCoord;
}
