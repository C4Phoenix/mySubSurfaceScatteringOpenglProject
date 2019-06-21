#version 330
layout(location=0) in vec3 InputPosition;
layout(location=1) in vec3 InputColor;
layout(location=2) in vec2 InputTexcoord;
layout(location=3) in vec3 InputNormal;
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

out vec3 normal;
out vec3 color;
out vec2 texCoord;

void main()
{
/*
	color = InputColor;
	texCoord = InputTexcoord;

	mat3 normalMatrix = mat3(modelViewMatrix);
	normalMatrix = transpose(inverse(normalMatrix));
	normal = normalMatrix * a_normal;
	
	gl_Position = projectionMatrix * modelViewMatrix * vec4(a_position,1);
	*/
}