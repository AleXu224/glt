#version 450
layout(binding = 0) uniform Ubo {
    mat4 model;
	mat4 view;
}
ubo;

layout(location = 0) in vec4 inMargins;
layout(location = 1) in vec4 inPaddings;
layout(location = 2) in vec2 inSize;
layout(location = 3) in vec2 inPos;
layout(location = 4) in vec2 inUv;

layout(location = 0) out vec4 fragMargins;
layout(location = 1) out vec4 fragPaddings;
layout(location = 2) out vec2 fragSize;
layout(location = 3) out vec2 fragUv;

void main() {
	vec2 pos = inPos + inUv * inSize;
	gl_Position = ubo.view * ubo.model * vec4(pos, 1.0, 1.0);
	fragMargins = inMargins;
	fragPaddings = inPaddings;
	fragSize = inSize;
	fragUv = inUv;
}