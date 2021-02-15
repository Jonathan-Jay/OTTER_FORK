#version 420

layout(location = 0) in vec2 inUV;

out vec4 bright_color;

layout (binding = 0) uniform sampler2D s_screenTex;

uniform float u_Threshold;

//done with the guidance of https://learnopengl.com/Advanced-Lighting/Bloom
void main() 
{
	vec3 colour = texture(s_screenTex, inUV).rgb;

	float brightness = dot(colour, vec3(0.2126, 0.7152, 0.0722));
	if (brightness > u_Threshold)
		bright_color = vec4(colour, 1.0);
	else
		bright_color = vec4(0.0, 0.0, 0.0, 1.0);
}