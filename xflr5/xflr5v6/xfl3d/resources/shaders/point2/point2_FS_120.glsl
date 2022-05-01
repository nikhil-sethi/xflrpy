#version 120

uniform float clipPlane0; // defined in view-space

in vec3 Position_viewSpace;
in vec4 pointcolor;



void main(void)
{
    if (Position_viewSpace.z > clipPlane0)
    {
        discard;
        return;
    }

    vec2 coord = gl_PointCoord - vec2(0.5);  //from [0,1] to [-0.5,0.5]
    if(length(coord) > 0.5)                  //outside of circle radius?
    {
        discard;
        return;
    }

    gl_FragColor = vec4(pointcolor.xyz, 1.0-length(coord)*2.0);
//    fragColor = pointcolor;
}
