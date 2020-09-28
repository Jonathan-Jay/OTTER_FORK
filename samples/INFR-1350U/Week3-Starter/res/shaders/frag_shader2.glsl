#version 410

layout(location = 1) in vec3 inColor;

out vec4 frag_color;

void main() { 
	
	frag_color = vec4(inColor, 1.0) * vec4(3.0, 0.25, 0.25, 1.0);
}