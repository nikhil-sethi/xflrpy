#version 330

in vec3 Position_viewSpace;
in vec3 Normal_viewSpace;
in vec3 EyeDirection_viewSpace;
in vec3 LightDirection_viewSpace;
in vec3 vPosition;
in vec4 vertexcolor;

uniform int lightOn;
uniform vec3 LightPosition_viewSpace;
uniform vec4 LightColor;
uniform float LightAmbient, LightDiffuse, LightSpecular;
uniform float Kc, Kl, Kq;
uniform int MaterialShininess;
uniform vec4 clipPlane0; // defined in view-space



layout(location=0) out vec4 fragColor;


void main()
{
    // Material properties
    vec4 MaterialAmbientColor, MaterialDiffuseColor, MaterialSpecularColor;

    if (vPosition.z > clipPlane0.w)
    {
        discard;
        return;
    }

    if(lightOn==1)
    {
        MaterialAmbientColor  = vec4(vertexcolor.rgb * LightAmbient, vertexcolor.a);
        MaterialDiffuseColor  = vec4(vertexcolor.rgb * LightDiffuse, vertexcolor.a);
        MaterialSpecularColor = vec4(1.0, 1.0, 1.0, 1.0);

        // Distance to the light
        float distance = length(LightPosition_viewSpace - Position_viewSpace);

        // Normal of the computed fragment, in viewSpace
        vec3 N = normalize(Normal_viewSpace);

        // Direction from the fragment to the light
        vec3 L = normalize(LightDirection_viewSpace);

        // Cosine of the angle between the normal and the light direction, clamped above 0
        float cosTheta = clamp(dot(N,L), 0.0, 1.0);

        // Direction from the vertex to the eye, in view space.
        vec3 E = normalize(EyeDirection_viewSpace);

        // Direction in which the triangle reflects the light
        vec3 R = reflect(-L,N);

        // Cosine of the angle between the Eye vector and the Reflect vector,
        float cosAlpha = clamp(dot(E,R), 0.0, 1.0);

        float attenuation_factor = clamp(1.0/(Kc + Kl*distance + Kq*distance*distance), 0.00001, 1.0);

        fragColor =
              MaterialAmbientColor  * LightColor +
             (MaterialDiffuseColor  * LightDiffuse  * cosTheta)                         * LightColor * attenuation_factor
            +(MaterialSpecularColor * LightSpecular * pow(cosAlpha, MaterialShininess)) * LightColor * attenuation_factor;
    }
    else
    {
        fragColor  = vertexcolor;
    }
}
















