#version 120

varying vec2 fTexCoord;

uniform sampler2D sCharacters;
uniform vec4 color;

void main()
{
	gl_FragColor = vec4(texture2D(sCharacters, fTexCoord).r) * color;
}
