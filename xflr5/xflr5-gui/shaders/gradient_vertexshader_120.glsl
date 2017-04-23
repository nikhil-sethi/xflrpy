#version 120

attribute vec4 vertexPosition_modelSpace;
attribute vec4 vertexColor;

varying vec4 outColor;

uniform mat4 pvmMatrix;

void main()
{
   outColor = vertexColor;
   gl_Position = pvmMatrix * vertexPosition_modelSpace;
}
