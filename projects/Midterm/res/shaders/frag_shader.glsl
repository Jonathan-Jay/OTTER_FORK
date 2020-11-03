#version 410
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;

uniform vec3 colour;

uniform vec3  lightPos1;
uniform vec3  lightPos2;
uniform vec3  lightColour;

uniform float specularStrength;
uniform float shininess;

uniform float ambientLightStrength;
uniform vec3  ambientColour;
uniform float ambientStrength;

out vec4 frag_color;

void main() {
	// Lecture 5
	vec3 ambient = ((ambientLightStrength * lightColour) + (ambientColour * ambientStrength));

	// Diffuse
	vec3 N = normalize(inNormal);
	vec3 lightDir1 = normalize(lightPos1 - inPos);
	vec3 lightDir2 = normalize(lightPos2 - inPos);

	float dif = max(dot(N, lightDir1), 0.0) + max(dot(N, lightDir2), 0.0);
	vec3 diffuse = dif * lightColour;// add diffuse intensity

	//Attenuation
	float dist1 = length(lightPos1 - inPos);
	float dist2 = length(lightPos2 - inPos);
	diffuse = diffuse / dist1 + diffuse / dist2; // (dist*dist)

	// Specular
	float spec1 = pow(max(dot(N, lightDir1), 0.0), shininess); // Shininess coefficient (can be a uniform)
	vec3 specular1 = specularStrength * spec1 * lightColour; // Can also use a specular color
	float spec2 = pow(max(dot(N, lightDir2), 0.0), shininess); // Shininess coefficient (can be a uniform)
	vec3 specular2 = specularStrength * spec2 * lightColour; // Can also use a specular color

	vec3 result = (ambient + diffuse + specular1 + specular2) * colour;

	frag_color = vec4(result, 1.0);
}