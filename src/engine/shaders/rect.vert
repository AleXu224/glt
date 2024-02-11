#version 450
layout(binding = 0) uniform Ubo {
	mat4 view;
}
ubo;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec4 inBorderColor;
layout(location = 2) in vec4 inBorderRadiuses;
layout(location = 3) in vec4 inBorderSizes;
layout(location = 4) in vec2 inSize;
layout(location = 5) in vec2 inPos;
layout(location = 6) in vec2 inUv;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 fragBorderColor;
layout(location = 2) out vec2 fragUv;
layout(location = 3) out vec2 fragSize;
layout(location = 4) out vec4 fragBorderSizes;
layout(location = 5) out vec4 fragBorderRadiuses;


void main() {
	vec2 pos = inPos + inUv * inSize;
	gl_Position = ubo.view * vec4(pos, 1.0, 1.0);
	fragColor = inColor;
	fragBorderColor = inBorderColor;
	fragUv = inUv;
	fragSize = inSize;
	fragBorderRadiuses = inBorderRadiuses;
	fragBorderSizes = inBorderSizes;
}