#version 330

// the texture fragment shader

uniform int lightOn;
uniform vec3 LightPosition_viewSpace;
uniform vec4 LightColor;
uniform float LightAmbient, LightDiffuse, LightSpecular;
uniform float Kc, Kl, Kq;
uniform int MaterialShininess;
uniform sampler2D textureSampler;
uniform vec4 clipPlane0; // defined in view-space

in vec3 Position_worldSpace;
in vec3 Normal_viewSpace;
in vec3 EyeDirection_viewSpace;
in vec3 LightDirection_viewSpace;
in vec2 UV;
in vec3 vPosition;

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
        MaterialAmbientColor  = vec4(texture(textureSampler, UV).rgb*LightAmbient, 1.0);
        MaterialDiffuseColor  = vec4(texture(textureSampler, UV).rgb*LightDiffuse, 1.0);


        MaterialSpecularColor = vec4(1.0, 1.0, 1.0, 1.0);

        // Distance to the light
        float distance = length(LightPosition_viewSpace - Position_worldSpace);

        // Normal of the computed fragment, in camera space
        vec3 N = normalize(Normal_viewSpace);

        // Direction of the light (from the fragment to the light)
        vec3 L = normalize(LightDirection_viewSpace);

        // Cosine of the angle between the normal and the light direction,
        float cosTheta = clamp(dot(N,L), 0.0, 1.0);

        // Eye vector (towards the camera)
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
        fragColor  = vec4(texture(textureSampler, UV).rgb, 1.0);

}



