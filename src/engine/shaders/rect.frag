#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec4 fragBorderColor;
layout(location = 2) in vec2 fragUv;
layout(location = 3) in vec2 fragSize;
layout(location = 4) in vec4 fragBorderSizes;
layout(location = 5) in vec4 fragBorderRadiuses;

layout(location = 0) out vec4 outColor;

float borderDistance(vec2 coords, vec2 halfRes, vec2 radius) {
	radius = max(radius, vec2(0));
	vec2 p2 = (coords - (halfRes - radius));
	vec2 p2norm = p2 / radius;
	float h = length(p2norm);
	float sine = p2norm.y / h;
	float cosine = p2norm.x / h;
	vec2 p3 = vec2(radius.x * cosine, radius.y * sine);
	float len2 = length(p2);
	float len3 = length(p3);
	float distCorner = len2 - len3;

	vec2 posTop = vec2(clamp(coords.x, 0.f, halfRes.x - radius.x), halfRes.y);
	vec2 posRight = vec2(halfRes.x, clamp(coords.y, 0.f, halfRes.y - radius.y));
	float distTop = length(posTop - coords);
	float distRight = length(posRight - coords);
	float edgeDist =
		distTop < distRight ? coords.y - halfRes.y : coords.x - halfRes.x;

	float finalDist = p2.x < 0.f || p2.y < 0.f ? edgeDist : distCorner;
	return finalDist;
}

void main() {
	outColor = vec4(0.f);
	// Border size order: top, right, bottom, left
	vec2 borderSize = vec2(fragUv.x < 0.5 ? fragBorderSizes.w : fragBorderSizes.y, fragUv.y < 0.5 ? fragBorderSizes.x : fragBorderSizes.z);
	// Border radius order: topleft, topright, bottomright, bottomleft
	float borderRadius = fragUv.x < 0.5 ? (fragUv.y < 0.5 ? fragBorderRadiuses.x : fragBorderRadiuses.w) : (fragUv.y < 0.5 ? fragBorderRadiuses.y
																														: fragBorderRadiuses.z);

	vec2 coords = (fragUv * fragSize);
	vec2 halfSize = fragSize / 2.f;
	float dist = borderDistance(abs(coords - halfSize), halfSize, vec2(borderRadius));
	float dist2 = borderDistance(abs(coords - halfSize) + borderSize, halfSize, vec2(borderRadius) - borderSize);

	// These would only be needed in a 3d context.
	// In 2d the result will always be 1
	// vec2 grad_dist = vec2(dFdx(dist), dFdy(dist));
	float afwidth = 0.5f /* * length(grad_dist) */;
	float bdCov = smoothstep(afwidth, -afwidth, dist);

	// vec2 grad_dist2 = vec2(dFdx(dist2), dFdy(dist2));
	float afwidth2 = 0.5f /* * length(grad_dist2) */;
	float bgCov = smoothstep(afwidth2, -afwidth2, dist2);

	// Paint border
	if (borderSize.x != 0.f || borderSize.y != 0.f)
		outColor = mix(outColor, vec4(fragBorderColor.rgb * fragBorderColor.a, fragBorderColor.a), bdCov);
	// Paint background
	outColor = mix(outColor, vec4(fragColor.xyz * fragColor.a, fragColor.a), bgCov);
}