#version 120

uniform int pattern;   // an integer betwween 0 and 0xFFFF representing the bitwise pattern
uniform int nPatterns; // the number of patterns/unit length of the viewport, typically 200-300 for good pattern density
uniform float clipPlane0; // defined in view-space
uniform int HasUniColor = 1;
uniform vec4 UniformColor;

varying float texCoord;
varying vec3 vPosition;
varying vec4 GeomColor;

void main(void)
{
    if (vPosition.z > clipPlane0) {
        discard;
        return;
    }

    if(HasUniColor==1)
        gl_FragColor = UniformColor;
    else
        gl_FragColor = GeomColor; // from the Geometry Shader
}
