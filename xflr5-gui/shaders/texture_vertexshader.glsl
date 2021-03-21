#version 330

// the texture vertex shaders

// Input vertex data, different for all executions of this shader.
in vec4 vertexPosition_modelSpace;
in vec3 vertexNormal_modelSpace;
in vec2 vertexUV;

uniform  vec4 incolor;

// Values that stay constant for the whole mesh.
uniform mat4 pvmMatrix;
uniform mat4 vMatrix;
uniform mat4 mMatrix;

uniform vec3 LightPosition_viewSpace;
uniform vec3 EyePosition_viewSpace;

// Output data; will be interpolated for each fragment.
out vec3 Position_worldSpace;
out vec3 Normal_viewSpace;
out vec3 EyeDirection_viewSpace;
out vec3 LightDirection_viewSpace;
out vec4 vertexcolor;
out vec2 UV;
out vec3 vPosition;


void main()
{
    // Output position of the vertex, in clip space : MVP * position
    gl_Position =  pvmMatrix * vertexPosition_modelSpace;

    vec4 vsPos = vMatrix * mMatrix * vertexPosition_modelSpace; // position of vertex in viewspace

    vPosition = vsPos.xyz / vsPos.w;

    // Position of the vertex, in worldspace : M * position
    Position_worldSpace = (mMatrix * vertexPosition_modelSpace).xyz;

    // Vector that goes from the vertex to the eye, in view space.
    // In view space, the eye is at the origin (0,0,0).
    EyeDirection_viewSpace = EyePosition_viewSpace - vsPos.xyz;

    // Vector that goes from the vertex to the light, in view space.
//    vec3 LightPosition_viewSpace = ( vMatrix * vec4(LightPosition_viewSpace,1)).xyz;
    LightDirection_viewSpace = LightPosition_viewSpace - vsPos.xyz;

    // Normal of the the vertex, in camera space
    Normal_viewSpace = (vMatrix * mMatrix * vec4(vertexNormal_modelSpace,0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.

    vertexcolor = incolor;

    UV = vertexUV;
}



