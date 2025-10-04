#version 120

uniform mat4 pvmMatrix;
uniform mat4 vmMatrix;

attribute vec4 vertexPosition_modelSpace;
attribute vec4 vertexColor;

varying vec4 GeomColor;
varying vec3 vPosition;

void main(void)
{
    GeomColor = vertexColor;
    gl_Position =  pvmMatrix * vertexPosition_modelSpace;// pass through, the processing is done in the geom shader

    vec4 vsPos = vmMatrix * gl_Position;

    vPosition = vsPos.xyz / vsPos.w;
}
