/****************************************************************************

    xfl5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#ifdef GL_ES
precision mediump float;
#endif

varying vec3 Position_viewSpace;
varying vec3 Normal_viewSpace;
varying vec2 UV;
varying vec4 FragPosLightSpace;
varying vec4 VSColor;

uniform int HasUniColor = 1; // otherwise the attribut color will be used
uniform vec4 UniformColor;
uniform int HasTexture = 0;
uniform int HasShadow = 0;
uniform int TwoSided;
uniform int LightOn;
uniform vec3 LightPosition_viewSpace;
uniform vec3 EyePosition_viewSpace;
uniform sampler2D TheSampler;

uniform vec4 LightColor;
uniform float LightAmbient, LightDiffuse, LightSpecular;
uniform float Kc, Kl, Kq;
uniform float MaterialShininess;
uniform float clipPlane0; // defined in view-space

float ShadowCalculation(vec4 fragPosLightSpace)
{
    return 0.0;
}


void main()
{
    if (Position_viewSpace.z > clipPlane0)
    {
        discard;
        return;
    }

    vec4 fragcolor;
    if(HasUniColor==1) fragcolor = UniformColor;
    else               fragcolor = VSColor; // incoming from the Vertex Shader


    if(LightOn==1)
    {
        // Material properties
        vec4 MaterialAmbientColor, MaterialDiffuseColor, MaterialSpecularColor;
        if(HasTexture==1)
        {
        }
        else
        {
            MaterialAmbientColor  = vec4(fragcolor.rgb * LightAmbient, fragcolor.a);
            MaterialDiffuseColor  = vec4(fragcolor.rgb * LightDiffuse, fragcolor.a);
            MaterialSpecularColor = vec4(1.0, 1.0, 1.0, 1.0);
        }

        // Vector that goes from the vertex to the eye, in view space.
        vec3 EyeDirection_viewSpace = EyePosition_viewSpace - Position_viewSpace;

        // Vector that goes from the vertex to the light, in view space.
        vec3 LightDirection_viewSpace = LightPosition_viewSpace - Position_viewSpace;

        // Distance to the light
        float distance = length(LightPosition_viewSpace - Position_viewSpace);

        // Normal of the computed fragment, in viewSpace
        vec3 N = normalize(Normal_viewSpace);

        // Direction from the fragment to the light
        vec3 L = normalize(LightDirection_viewSpace);

        // Cosine of the angle between the normal and the light direction, clamped above 0
        float cosTheta = 1.0;
        if(TwoSided==0) cosTheta = clamp(dot(N,L), 0.0, 1.0);
        else            cosTheta = abs(dot(N,L)); // reflection on both sides of the surface

        // Direction from the vertex to the eye, in view space.
        vec3 E = normalize(EyeDirection_viewSpace);

        // Direction in which the triangle reflects the light
        vec3 R = reflect(-L,N);

        // Cosine of the angle between the Eye vector and the Reflect vector,
        float cosAlpha = clamp(dot(E,R), 0.0, 1.0);

        float attenuation_factor = clamp(1.0/(Kc + Kl*distance + Kq*distance*distance), 0.00001, 1.0);

        if(HasShadow==0)
        {
            gl_FragColor =
                      MaterialAmbientColor  * LightColor +
                     (MaterialDiffuseColor  * LightDiffuse  * cosTheta)                         * LightColor * attenuation_factor
                    +(MaterialSpecularColor * LightSpecular * pow(cosAlpha, MaterialShininess)) * LightColor * attenuation_factor;
        }
        else
        {
            float shadow = ShadowCalculation(FragPosLightSpace);
    //        float shadow = 0.0;
            gl_FragColor =
                      MaterialAmbientColor  * LightColor +
                     (MaterialDiffuseColor  * LightDiffuse  * cosTheta)                         * LightColor * attenuation_factor * (1.0-shadow)
                    +(MaterialSpecularColor * LightSpecular * pow(cosAlpha, MaterialShininess)) * LightColor * attenuation_factor * (1.0-shadow);
        }
    }
    else
    {
//        if(HasTexture==0)
            gl_FragColor  = fragcolor;
//        else gl_FragColor  = vec4(texture(TheSampler, UV).rgb, 1.0);
    }
}














