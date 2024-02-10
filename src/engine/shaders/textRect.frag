#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUv;

layout(set = 1, binding = 0) uniform sampler2D tex;

layout(location = 0) out vec4 outColor;

void main() {
	float alpha = texture(tex, fragUv).r * fragColor.a;
	outColor = vec4(fragColor.xyz * alpha, alpha);
}