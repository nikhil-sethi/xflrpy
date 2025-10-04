/****************************************************************************
 *
 * 	xfl5 v6
 * 	Copyright (C) André Deperrois 
 * 	GNU General Public License v3
 *
 *****************************************************************************/

#ifdef GL_ES
precision mediump float;
#endif

// Input vertex data, different for all executions of this shader.
attribute vec4 vertexPosition_modelSpace;
attribute vec3 vertexNormal_modelSpace;
attribute vec2 vertexUV;
attribute vec4 vertexColor;

in vec3 vertexOffset; // used if instanced

uniform mat4 pvmMatrix;
uniform mat4 vmMatrix;
uniform mat4 LightViewMatrix;
uniform int Instanced = 0;
uniform float uScale;  // used if instanced


// Output data; will be interpolated for each fragment.
varying vec3 Position_viewSpace;
varying vec3 Normal_viewSpace;
varying vec2 UV;
varying vec4 FragPosLightSpace;
varying vec4 VSColor;


void main()
{
    vec4 vertexpos = vertexPosition_modelSpace;

    if(Instanced==1)
    {
        // scale first
        vertexpos.x = vertexPosition_modelSpace.x*uScale;
        vertexpos.y = vertexPosition_modelSpace.y*uScale;
        vertexpos.z = vertexPosition_modelSpace.z*uScale;
        vertexpos.w = vertexPosition_modelSpace.w;
        vertexpos += vec4(vertexOffset, 0.0);    // then translate
    }

    // Output position of the vertex, in clip space : MVP * position
    gl_Position =  pvmMatrix * vertexpos;

    vec4 vsPos = vmMatrix * vertexpos; // position of vertex in viewspace

    Position_viewSpace = vsPos.xyz;
    // Normal of the the vertex, in camera space
    // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
    Normal_viewSpace = vec3(vmMatrix * vec4(vertexNormal_modelSpace,0));

    // the vertex color, optional
    VSColor = vertexColor;

    // in case there is a texture
    UV = vertexUV;

    // shadow addition
    FragPosLightSpace = LightViewMatrix * vertexpos;
}



