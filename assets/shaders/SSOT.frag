#version 400

// Input variables
in vec2 FragTexcoord;

uniform sampler2D f2dFar;
uniform sampler2D f2dClose;
uniform vec2 vViewportSize;
// Output variables
out vec4 OutputColor;

float getDepth(sampler2D image, vec2 uv)
{
  float scale = 3;
  return texture2D(image ,uv).x*scale;
}

float depthCalc(float fClose, float fFar)
{
  float fScale= 15;// setting to controll how noticable the differance is
  float depth =1 - (fFar - fClose);
  depth = pow(depth+1,fScale)/pow(1+1,fScale);
  depth = clamp(depth, 0.2, 1.0);// set max depth
  return depth;
}

float blur13(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  float depth = .0;
  vec2 off1 = vec2(1.411764705882353) * direction;
  vec2 off2 = vec2(3.2941176470588234) * direction;
  vec2 off3 = vec2(5.176470588235294) * direction;
  depth += getDepth(image, uv) * 0.1964825501511404;
  depth += getDepth(image, uv + (off1 / resolution)) * 0.2969069646728344;
  depth += getDepth(image, uv - (off1 / resolution)) * 0.2969069646728344;
  depth += getDepth(image, uv + (off2 / resolution)) * 0.09447039785044732;
  depth += getDepth(image, uv - (off2 / resolution)) * 0.09447039785044732;
  depth += getDepth(image, uv + (off3 / resolution)) * 0.010381362401148057;
  depth += getDepth(image, uv - (off3 / resolution)) * 0.010381362401148057;
  return depth;
}

void main(void)
{
  vec2 UV = FragTexcoord;
  vec2 vDirection = vec2(1,0);
  float ffar = blur13(f2dFar, UV, vViewportSize, vDirection);// read buf
  float fClose = blur13(f2dClose, UV, vViewportSize, vDirection);// read buf
  float fThicknes =depthCalc(fClose,ffar);
  OutputColor = vec4(fThicknes,fThicknes,fThicknes,1);//vec4(1,1,1,1);
}
