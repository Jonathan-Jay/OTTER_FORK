#version 410
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColour;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inSpec;

uniform sampler2D s_texture;

uniform vec3  lightPos1;
uniform vec3  lightPos2;
uniform vec3  lightColour;

uniform float ambientLightStrength;
uniform vec3  ambientColour;
uniform float ambientStrength;

out vec4 frag_color;

void main() {
	// Lecture 5
	vec3 ambient = ((ambientLightStrength * lightColour) + (ambientColour * ambientStrength));

	vec3 N = normalize(inNormal);
	
	// Diffuse 1
	vec3 lightDir1 = normalize(lightPos1 - inPos);
	float spec1 = pow(max(dot(N, lightDir1), 0.0), inSpec.y);

	// Diffuse 2
	vec3 lightDir2 = normalize(lightPos2 - inPos);
	float spec2 = pow(max(dot(N, lightDir2), 0.0), inSpec.y);

	// Specular
	vec3 specular = inSpec.x * (spec1 + spec2) * lightColour;
	
	float diff = max(dot(N, lightDir1), 0.0) + max(dot(N, lightDir2), 0.0);
	vec3 diffuse = diff * lightColour;
	diffuse = diffuse / length(lightPos1 - inPos) + diffuse / length(lightPos2 - inPos);

	vec4 textureColour = texture(s_texture, inUV);
	vec3 result = (ambient + diffuse + specular) * inColour * textureColour.rgb;

	frag_color = vec4(result, textureColour.a * inSpec.z);
}