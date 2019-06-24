#version 400

// inspired by: https://colinbarrebrisebois.com/2011/03/07/gdc-2011-approximating-translucency-for-a-fast-cheap-and-convincing-subsurface-scattering-look/

//uniform sampler2D DepthTex;
//uniform sampler2D SSOT;
uniform float fLTScale;// scale value. direct / back translucency
uniform int iLTPower;  // Power value  for direct translucency. Breaks continuity, view-dependent. 2 everything is very translucent 12 not so much.
uniform vec3 vViewDirection;

uniform vec2 vWindowWidth;

uniform vec3 vEye;
uniform float fAmbientScale;// ambient light visible from all angles.

uniform int iLightCount;

uniform vec3 [20]vLightLocation;
uniform float [20]fLTattenuation;
uniform vec4 [20]vLTColour;

uniform sampler2D SSOT;

// Input variables
in vec2 FragTexcoord;
in vec3 FragNormal;
in vec4 FragColor;
in vec3 FragInputPosition;

// Output variables
out vec4 OutputColor;

float saturate(float x){ return clamp(x, 0.0, 1.0);}

float rand(vec2 c){
  return fract(sin(dot(c.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// start of stolen' noise from : https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
//  Classic Perlin 2D Noise 
//  by Stefan Gustavson
//
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec2 fade(vec2 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);}

float cnoise(vec2 P){
  vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
  vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
  Pi = mod(Pi, 289.0); // To avoid truncation effects in permutation
  vec4 ix = Pi.xzxz;
  vec4 iy = Pi.yyww;
  vec4 fx = Pf.xzxz;
  vec4 fy = Pf.yyww;
  vec4 i = permute(permute(ix) + iy);
  vec4 gx = 2.0 * fract(i * 0.0243902439) - 1.0; // 1/41 = 0.024...
  vec4 gy = abs(gx) - 0.5;
  vec4 tx = floor(gx + 0.5);
  gx = gx - tx;
  vec2 g00 = vec2(gx.x,gy.x);
  vec2 g10 = vec2(gx.y,gy.y);
  vec2 g01 = vec2(gx.z,gy.z);
  vec2 g11 = vec2(gx.w,gy.w);
  vec4 norm = 1.79284291400159 - 0.85373472095314 * 
    vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11));
  g00 *= norm.x;
  g01 *= norm.y;
  g10 *= norm.z;
  g11 *= norm.w;
  float n00 = dot(g00, vec2(fx.x, fy.x));
  float n10 = dot(g10, vec2(fx.y, fy.y));
  float n01 = dot(g01, vec2(fx.z, fy.z));
  float n11 = dot(g11, vec2(fx.w, fy.w));
  vec2 fade_xy = fade(Pf.xy);
  vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
  float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
  return 2.3 * n_xy;
}
// end of 'stolen' noise

// taken from: https://github.com/glslify/glsl-diffuse-oren-nayar
//Computes the diffuse intensity in the Lambertian model
//
//    lightDir is a unit length vec3 pointing from the surface point toward the light
//    viewDir is a unit length vec3 pointing toward the camera
//    normal is the unit length surface normal at the sample point
//    roughness is a parameter between 0 and 1 measuring the surface roughness. 0 for smooth, 1 for matte
//    albedo measures the intensity of the diffuse reflection. Values >0.96 do not conserve energy
//Returns A float representing the diffuse light intensity
#define PI 3.14159265
float orenNayarDiffuse(
  vec3 lightDirection,
  vec3 viewDirection,
  vec3 surfaceNormal,
  float roughness,
  float albedo) {
  
  float LdotV = dot(lightDirection, viewDirection); // L \ ? / V
  float NdotL = dot(lightDirection, surfaceNormal); // L \ ? | N
  float NdotV = dot(surfaceNormal, viewDirection);  // N | ? / V

  float s = LdotV - NdotL * NdotV; //+ texIntensity;
  // LdotV max that is able to reflect; NdotL * NdotV remove from that maximum by looking at the angle
 
  float t = mix(1.0, max(NdotL, NdotV), step(0.0, s));

  //roughness probablity function 
  float sigma2 = roughness * roughness;
  float A = 1.0 + sigma2 * (albedo / (sigma2 + 0.13) + 0.5 / (sigma2 + 0.33));
  float B = 0.45 * sigma2 / (sigma2 + 0.09);
  return saturate(albedo / PI// max shining value 
          * max(0.0, NdotL) // bekijk of er naar de onderkant wordt gekeken
          * ( A + B 
             * s / t));
}


void main(void)
{
  float debugFlaot;
  vec2 UV = FragTexcoord;
  vec3 vNormal = normalize(FragNormal);
  vec4 C = FragColor;
  vec3 vViewDirection = normalize(vViewDirection);normalize(vEye);
  //float fLThickness = length( texture2D(DepthTex, FragTexcoord));//0.2 added for more light to go thru (bad thickness texture)
  
  vec2 texcoord = gl_FragCoord.xy / vWindowWidth;
  vec4 vSSOT = texture(SSOT, texcoord);
  //float fLThickness =.8;
  float fLThickness = vSSOT.x;
  debugFlaot =fLThickness;
  /*
  // subsurface distortion. shifts the suface normal. breaks continuity.
  float fLightDistortion = cnoise(FragTexcoord*800)*.05 ;
  // ambient light
  vec3 fLight = vec3(1,1,1) * fLThickness * fAmbientScale;// ambient light

  //per light
  for(int i =0; i< iLightCount; i++)
  {
    vec3 vLightDirection = vLightLocation[i] - FragInputPosition;// not yet normalized becouse of distance calc
    float fLightDistance = length(vLightDirection);  
    vLightDirection = normalize(vLightDirection);
  
    // amount by wich the light strenght dissapates between 0 and 1;
    float Attenuation = saturate( 1.0 / (1.0 + fLTattenuation[i] * pow(fLightDistance, 2)));
    vec3 fLightAttenuation = vLTColour[i].xyz *saturate( Attenuation);
  
    //sub_surface_scatter
    vec3 vLTLight = vLightDirection + vNormal * fLightDistortion;
    float fLightPoint = saturate(dot(normalize(vEye), normalize(-vLTLight)));
    float fLightStrenth = pow(fLightPoint , iLTPower) * fLTScale;
    float fLightObjectScater = fLightStrenth * fLThickness;

    float fDiffuse = orenNayarDiffuse(
      vLightDirection,
      vViewDirection,
      normalize(vNormal * .5+ fLightDistortion),
      .99,
      Attenuation
    );
    
    fLight = fLight + vLTColour[i].xyz* fDiffuse+ vec3( fLightAttenuation* fLightObjectScater);
  }
  
  OutputColor = vec4( fLight * C.xyz,1);
  */
  OutputColor = vec4(vec3(1,1,1)* debugFlaot,1);
}
