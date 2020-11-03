#version 410
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

uniform vec3 colour;
uniform vec3 camPos;

uniform vec3  lightPos;
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
	vec3 lightDir = normalize(lightPos - inPos);

	float dif = max(dot(N, lightDir), 0.0);
	vec3 diffuse = dif * lightColour;// add diffuse intensity

	//Attenuation
	float dist = length(lightPos - inPos);
	diffuse = diffuse / dist; // (dist*dist)

	// Specular
	vec3 camDir = normalize(camPos - inPos);
	vec3 reflectDir = reflect(-lightDir, N);
	float spec = pow(max(dot(camDir, reflectDir), 0.0), shininess); // Shininess coefficient (can be a uniform)
	vec3 specular = specularStrength * spec * lightColour; // Can also use a specular color

	vec3 result = (ambient + diffuse + specular) * colour;

	frag_color = vec4(result, 1.0);
}