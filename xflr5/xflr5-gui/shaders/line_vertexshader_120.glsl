#version 120

attribute vec4 vertex;
uniform mat4 pvmMatrix;
uniform mat4 mMatrix;
uniform mat4 vMatrix;

varying vec3 vPosition;

void main(void)
{
	gl_Position = pvmMatrix * vertex;
	vec4 vsPos = vMatrix * mMatrix * vertex; // position of vertex in viewspace

	vPosition = vsPos.xyz / vsPos.w;
}
