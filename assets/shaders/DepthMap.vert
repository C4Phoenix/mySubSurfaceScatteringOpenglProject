#version 400

// Uniform Variables
uniform mat4 Proj;
uniform mat4 modView;
// Input Variables
in vec3 InputPosition;

// Output Variables
out vec3 FragInputPosition;

void main(void)
{
  gl_Position = Proj * modView * vec4(InputPosition, 1.0f);
  FragInputPosition = InputPosition;
}
