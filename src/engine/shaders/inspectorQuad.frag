#version 450

layout(location = 0) in vec4 fragMargins;
layout(location = 1) in vec4 fragPaddings;
layout(location = 2) in vec2 fragSize;
layout(location = 3) in vec2 fragUv;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 usedColor;
	vec2 coord = fragUv * fragSize;
	if (coord.x >= fragMargins.w + fragPaddings.w &&
		coord.y >= fragMargins.x + fragPaddings.x &&
		coord.x < fragSize.x - (fragMargins.z + fragPaddings.z) &&
		coord.y < fragSize.y - (fragMargins.y + fragPaddings.y)) {
		usedColor = vec4(0.75, 0.75, 1.f, 0.25f);
	} else if (coord.x >= fragMargins.w &&
			   coord.y >= fragMargins.x &&
			   coord.x < fragSize.x - fragMargins.z &&
			   coord.y < fragSize.y - fragMargins.y) {
		usedColor = vec4(0.5f, 0.f, 1.f, 0.25f);
	} else {
		usedColor = vec4(1.f, 0.5f, 0.f, 0.25f);
	}
	outColor = vec4(usedColor.xyz * usedColor.a, usedColor.a);
}