#version 330
uniform sampler2D s_texture;
in vec2 texCoord;
in vec3 color;
in vec3 normal;

void main()
{
    float f = dot(normal, vec3(0,0,1));

    float fogFac =  pow(clamp(1.0 - (gl_FragCoord.z / gl_FragCoord.w) / 1500.0,0,1),4.0);
    vec4 fogColor = vec4(0, 0, 0, 1);

	gl_FragColor = mix(fogColor, clamp((0.5 + f) * vec4(color,1),0,1)*texture2D(s_texture, texCoord), fogFac);
}