#version 120

uniform vec4 color;
uniform vec4 clipPlane0; // defined in view-space

varying vec3 vPosition;

void main(void)
{
    if (vPosition.z > clipPlane0.w) {
      discard;
    } else {
        gl_FragColor = color;
    }
}
