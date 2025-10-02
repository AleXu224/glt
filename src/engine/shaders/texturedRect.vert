#version 450
layout(binding = 0) uniform Ubo {
	mat4 model;
	mat4 view;
}
ubo;

layout(location = 0) in vec2 inSize;
layout(location = 1) in vec2 inPos;
layout(location = 2) in vec2 inUv;

layout(location = 0) out vec2 fragUv;

void main() {
	vec2 pos = inPos + inUv * inSize;
	gl_Position = ubo.view * ubo.model * vec4(pos, 1.0, 1.0);
	fragUv = inUv;
}