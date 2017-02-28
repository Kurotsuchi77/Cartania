#version 120

varying vec2 fTexCoords;

uniform sampler2D sColor;

uniform vec4 color;

void main()
{
    gl_FragColor = texture2D(sColor, fTexCoords) * color;
}
