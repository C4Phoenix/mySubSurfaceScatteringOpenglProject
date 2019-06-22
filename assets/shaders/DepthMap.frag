#version 400

//uniform vec3 vEyePos;
//uniform float fNear;
//uniform float fFar;

// Input variables
//in vec3 FragInputPosition;

// Output variables
//out vec4 OutputColor;
out float depthBuffer;

void main(void)
{
  //float depth =gl_FragCoord.z;//1/clamp(length(FragInputPosition - vEyePos), fNear,fFar);
  //float depth =1/clamp(length(FragInputPosition - vEyePos), fNear,fFar);
  float depth = (gl_FragCoord.z / gl_FragCoord.w) / 10;
  //OutputColor = vec4(vec3(1,1,1) * depth,1);
  depthBuffer = depth;
}
