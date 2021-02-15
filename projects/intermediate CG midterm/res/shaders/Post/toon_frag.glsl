#version 420

layout(location = 0) in vec2 inUV;

out vec4 frag_color;

layout (binding = 0) uniform sampler2D s_screenTex;

const float divide = 7./12.;

uniform int bands;

void main() 
{
	vec4 source = texture(s_screenTex, inUV);

	float sum = (source.r + source.g + source.b) * divide;

	sum = floor(sum * bands) / bands;

	frag_color.rgb = source.rgb * sum;
	frag_color.a = source.a;
}