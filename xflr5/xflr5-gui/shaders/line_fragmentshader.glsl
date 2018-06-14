#version 330

uniform vec4 color;
uniform vec4 clipPlane0; // defined in view-space

in vec3 vPosition;

out vec4 fragColor;

void main(void)
{
	if (vPosition.z > clipPlane0.w)
	{
	  discard;
	}
	else
	{
		fragColor = color;
	}
}
