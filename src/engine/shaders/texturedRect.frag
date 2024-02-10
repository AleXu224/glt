#version 450

layout(location = 0) in vec2 fragUv;

layout(set = 1, binding = 0) uniform sampler2D tex;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 col = texture(tex, fragUv);
	outColor = vec4(col.xyz * col.a, col.a);
}