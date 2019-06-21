#version 400

// Uniform Variables
//uniform mat4 View;
uniform mat4 Proj;
uniform mat4 modView;
uniform mat3 normal;

//uniform float time;

layout (location = 0) in vec3 InputPosition;
layout (location = 1) in vec3 InputNormal;
layout (location = 2) in vec2 InputTexcoord;

// Output Variables
out vec3 FragNormal;
out vec4 FragColor;
out vec3 FragInputPosition;
out vec2 FragTexcoord;

void main(void)
{
  //gl_Position = Proj * View * vec4(InputPosition, 1.0f);
  gl_Position = Proj * modView * vec4(InputPosition, 1.0f);

  FragTexcoord = InputTexcoord;
  FragNormal = InputNormal;
  FragColor = vec4(1,1,1,1);//InputColor;
  FragInputPosition = InputPosition;
}
