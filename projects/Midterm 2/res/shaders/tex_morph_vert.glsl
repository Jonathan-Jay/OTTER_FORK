#version 410
layout(location = 0) in vec3 inPos1;
layout(location = 1) in vec3 inPos2;
layout(location = 2) in vec3 inNorm1;
layout(location = 3) in vec3 inNorm2;
layout(location = 4) in vec3 inCol1;
layout(location = 5) in vec3 inCol2;
layout(location = 6) in vec3 inSpec1;
layout(location = 7) in vec3 inSpec2;
layout(location = 8) in vec3 inUV;

uniform mat4 MVP;
uniform mat4 transform;

uniform float t;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outColour;
layout(location = 3) out vec3 outSpec;
layout(location = 4) out vec2 outUV;

void main() {
	// vertex position in clip space
	vec3 pos = mix(inPos1, inPos2, t);

	outPosition = (transform * vec4(pos, 1)).xyz;
	
	outNormal = mat3(transform) * mix(inNorm1, inNorm2, t);

	outColour = mix(inCol1, inCol2, t);

	outSpec = mix(inSpec1, inSpec2, t);

	outUV = inUV.xy;

	gl_Position = MVP * vec4(pos, 1);
}


