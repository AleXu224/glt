#version 450

layout(location = 0) in vec2 fragUv;
layout(location = 1) in vec2 texelSize;

layout(set = 1, binding = 0) uniform sampler2D tex;

layout(location = 0) out vec4 outColor;

void main() {
	vec2 origin = vec2(0.5);
	vec2 pos1 = vec2(0.125, 0.375);
	vec2 pos2 = vec2(0.625, 0.125);
	vec2 pos3 = vec2(0.375, 0.875);
	vec2 pos4 = vec2(0.875, 0.625);

	vec4 col = texture(tex, fragUv - (origin - pos1) * texelSize);
	col += texture(tex, fragUv - (origin - pos2) * texelSize);
	col += texture(tex, fragUv - (origin - pos3) * texelSize);
	col += texture(tex, fragUv - (origin - pos4) * texelSize);
	col *= 0.25;
	outColor = vec4(col.xyz * col.a, col.a);
}