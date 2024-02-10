#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec4 fragBorderColor;
layout(location = 2) in vec2 fragUv;
layout(location = 3) in vec2 fragSize;
layout(location = 4) in vec4 fragBorderSizes;
layout(location = 5) in vec4 fragBorderRadiuses;

layout(location = 0) out vec4 outColor;

float ellipseBorderDistance(vec2 coords, vec2 halfRes, vec2 borderRadius, vec2 inset) {
	// The border radius and newCoords variable are both offset by 0.5 pixels in order to be able to get antialiasing for free
	vec2 newBorderRadius = borderRadius - inset + 0.5;
	// In the case that the border radius is 0 on any side then the resulting output is a rectangle
	// This is needed since newCoords would divide by 0 otherwise
	if (newBorderRadius.x <= 0.f || newBorderRadius.y <= 0.f) {
		if (coords.x > (halfRes.x - inset.x) || coords.y > (halfRes.y - inset.y))
			return 1.f;
		else
			return 0.f;
	}
	vec2 newHalfRes = halfRes - inset;

	vec2 newCoords = (coords + 0.4 - (newHalfRes - newBorderRadius)) / newBorderRadius;
	// Fill out the rest of the space that isn't the 4 ellipses
	if ((newCoords.x <= 0.0 && newCoords.y <= 1.0) || (newCoords.y <= 0.0 && newCoords.x <= 1.0)) return 0.f;

	float h = length(newCoords);
	float sine = newCoords.y / h;
	float cosine = newCoords.x / h;

	vec2 borderPos = vec2(newBorderRadius.x * cosine, newBorderRadius.y * sine);
	vec2 translatedCoords = newCoords * (newBorderRadius);
	// Fill the inside of the ellipses
	if (translatedCoords.x <= borderPos.x && translatedCoords.y <= borderPos.y) return 0.f;
	return distance(translatedCoords, borderPos);
}

void main() {
	// Border sizes are as follows: top, right, bottom, left
	vec2 borderSize = vec2(
		fragUv.x < 0.5 ? fragBorderSizes.w : fragBorderSizes.y,
		fragUv.y < 0.5 ? fragBorderSizes.x : fragBorderSizes.z
	);
	// Border radiuses are as follows: topleft, topright, bottomright, bottomleft
	float borderRadius = fragUv.x < 0.5 ? fragUv.y < 0.5 ? fragBorderRadiuses.x : fragBorderRadiuses.w : fragUv.y < 0.5 ? fragBorderRadiuses.y : fragBorderRadiuses.z;

	vec2 coords = (fragUv * fragSize);
	vec2 halfRes = (0.5 * fragSize);

	vec4 retColor = vec4(0);
	if (borderSize.x > 0.0 || borderSize.y > 0.0) {
		float edgeDist = ellipseBorderDistance(abs(coords - halfRes), halfRes, vec2(borderRadius), vec2(0));
		retColor = mix(vec4(fragBorderColor.rgb * fragBorderColor.a, fragBorderColor.a), vec4(0.0), smoothstep(0.0, 1.0, edgeDist));
	}
	float edgeDist = ellipseBorderDistance(abs(coords - halfRes), halfRes, vec2(borderRadius), borderSize);
	outColor = mix(vec4(fragColor.xyz * fragColor.a, fragColor.a), retColor, smoothstep(0.0, 1.0, edgeDist));
}