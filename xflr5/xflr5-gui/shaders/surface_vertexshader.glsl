#version 330

// Input vertex data, different for all executions of this shader.
in vec4 vertexPosition_modelSpace;
in vec3 vertexNormal_modelSpace;

uniform vec4 incolor;
uniform mat4 pvmMatrix;
uniform mat4 vMatrix;
uniform mat4 mMatrix;
uniform vec4 clipPlane0; // defined in view-space
uniform vec3 LightPosition_viewSpace;
uniform vec3 EyePosition_viewSpace;

// Output data; will be interpolated for each fragment.
out vec3 Position_viewSpace;
out vec3 Normal_viewSpace;
out vec3 EyeDirection_viewSpace;
out vec3 LightDirection_viewSpace;
out vec3 vPosition;
out vec4 vertexcolor;


void main()
{
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  pvmMatrix * vertexPosition_modelSpace;

	vec4 vsPos = vMatrix * mMatrix * vertexPosition_modelSpace; // position of vertex in viewspace
//	gl_ClipDistance[0] = dot(clipPlane0,vsPos);
	vPosition = vsPos.xyz / vsPos.w;

	Position_viewSpace = vsPos.xyz;

	// Vector that goes from the vertex to the eye, in view space.
	EyeDirection_viewSpace = EyePosition_viewSpace - Position_viewSpace;

	// Vector that goes from the vertex to the light, in view space.
	LightDirection_viewSpace = LightPosition_viewSpace - Position_viewSpace;

	// Normal of the the vertex, in camera space
	// Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
	Normal_viewSpace = (vMatrix * mMatrix * vec4(vertexNormal_modelSpace,0)).xyz;

	vertexcolor = incolor;
}



