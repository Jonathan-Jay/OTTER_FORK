#version 420

layout(location = 0) in vec2 inUV;

out vec4 frag_colour;

layout (binding = 0) uniform sampler2D s_screenTex;
layout (binding = 1) uniform sampler2D s_brightTex;

uniform float u_Threshold;

//done with the guidance of https://learnopengl.com/Advanced-Lighting/Bloom
void main() 
{
	frag_colour = vec4(
		texture(s_screenTex, inUV).rgb
		+ texture(s_brightTex, inUV).rgb,
		1.0);
}