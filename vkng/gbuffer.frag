#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec3 viewCoord;
layout(location = 2) in vec3 viewNormal;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outColor;
layout(location = 3) out vec4 outStuff;

layout(binding = 1) uniform sampler2D tex;

void main() {
	outPosition = vec4(viewCoord*.5+.5,0.f);
	outNormal = vec4(viewNormal*.5+.5,1.f);
    outColor = texture(tex, texCoord);
	outStuff = gl_FragCoord;
}
