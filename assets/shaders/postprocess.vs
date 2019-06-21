#version 330
layout(location=0) in vec3 InputPosition;
out vec2 texCoord;

void main()
{
	texCoord = (vec2(InputPosition) + vec2(1, 1)) / 2.0;
	gl_Position = vec4(InputPosition,1);
}