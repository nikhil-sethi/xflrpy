#version 330

in vec4 outColor;

layout(location=0) out vec4 fragColor;

void main()
{
   fragColor = outColor;
}
