#version 120

attribute vec4 vCoords;

varying vec2 fTexCoord;

uniform vec2 position;

void main()
{
	gl_Position = vec4(vCoords.xy + position, 0.0, 1.0);
	fTexCoord = vCoords.zw;
}
