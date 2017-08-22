#version 450
#extension GL_ARB_separate_shader_objects : enable


out gl_PerVertex {
    vec4 gl_Position;
};

layout(location = 0) out vec2 texcoord;

void main() {
	texcoord = vec2(uvec2(gl_VertexIndex, gl_VertexIndex << 1) & 2); 
    gl_Position = vec4(mix(vec2(-1.0, -1.0), vec2(1.0, 1.0), texcoord), 0.0, 1.0);
}
