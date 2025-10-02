#version 450
layout(binding = 0) uniform Ubo {
    mat4 model;
	mat4 view;
}
ubo;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inSize;
layout(location = 2) in vec2 inPos;
layout(location = 3) in vec2 inOffset;
layout(location = 4) in vec2 inUv;
layout(location = 5) in vec2 inTextUv;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragUv;

void main() {
	vec2 pos = inPos + inUv * inSize + inOffset;
	gl_Position = ubo.view * ubo.model * vec4(pos, 1.0, 1.0);
	fragColor = inColor;
	fragUv = inTextUv;
}