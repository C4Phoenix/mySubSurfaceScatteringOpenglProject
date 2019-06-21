uniform sampler2D s_texture;
uniform float time;
in vec2 texCoord;
in vec3 color;


void main()
{
	gl_FragColor = mix(texture2D(s_texture, texCoord), texture2D(s_texture, texCoord + vec2(0.5, 0.5)), 0.5 + 0.5 * sin(time));
}