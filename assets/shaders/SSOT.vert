#version 400

// Uniform Variables
uniform mat4 View;
uniform mat4 Proj;

// Input Variables
layout(location=0) in vec3 InputPosition;
layout(location=1) in vec3 InputColor;
layout(location=2) in vec2 InputTexcoord;
layout(location=3) in vec3 InputNormal;

// Output Variables
out vec2 FragTexcoord;
out vec3 FragNormal;
out vec4 FragColor;
out vec3 FragTangent;
out vec3 FragBitangent;

void main(void)
{
  gl_Position = vec4(InputPosition,1);
  FragTexcoord = InputTexcoord;
  FragNormal = InputNormal;
}
