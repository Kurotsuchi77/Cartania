#version 120

attribute vec2 vPosition;
attribute vec2 vTexCoords;

varying vec2 fTexCoords;

uniform float oneOverNFrames, currFrame, mirror;

uniform vec2 res;

uniform vec2 position;
uniform vec2 size;

void main()
{
    fTexCoords  = vec2(currFrame*oneOverNFrames, 0.0) + vTexCoords * vec2(oneOverNFrames, 1.0);

    gl_Position = vec4(vPosition * vec2(mirror, 1.0) * (size*vec2(oneOverNFrames, 1.0)*res) + position, 0.0, 1.0);
}
