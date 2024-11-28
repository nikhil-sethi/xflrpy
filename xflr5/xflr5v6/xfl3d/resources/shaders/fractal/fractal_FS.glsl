#version 330

uniform int julia;
uniform float paramx; // the real part of the Julia set parameter
uniform float paramy; // the imag part of the Julia set parameter

uniform float tau;

uniform int maxiters;
uniform float maxlength;

in vec2 pos;

layout(location = 0) out vec4 FragmentColor;



float glGetRed(float tau)
{
    if     (tau>3.0f/4.0f) return 1.0f;
    else if(tau>1.0f/2.0f) return (4.0f*(tau-1.0f/2.0f));
    else                   return 0.0f;
}

float glGetGreen(float tau)
{
    if     (tau<0.0f || tau>1.0f) return 0.0f;
    else if(tau<1.0f/4.0f)        return (4.0f*tau);
    else if(tau>3.0f/4.0f)        return (1.0f-4.0f*(tau-3.0f/4.0f));
    else                          return 1.0;
}

float glGetBlue(float tau)
{
    if     (tau>1.0f/2.0f) return 0.0f;
    else if(tau>1.0f/4.0f) return (4.0f*(1.0f/2.0f-tau));
    else                   return 1.0f;
}


void main(void)
{
    float ptx=0.0;
    float pty=0.0;
    float ptx2=0.0;
    float pty2=0.0;
    float tmpx=0.0f;
    float length=0.0;
    float tlength=0.0;

    if(julia==1)
    {
        ptx=pos.x;
        pty=pos.y;
        ptx2=pos.x*pos.x;
        pty2=pos.y*pos.y;
    }

    int iter=0;
    do
    {
        ptx2 = ptx*ptx;
        pty2 = pty*pty;
        tmpx = ptx;
        if(julia==1)
        {
            ptx = ptx2 - pty2  + paramx;
            pty = 2.0*tmpx*pty + paramy;
        }
        else
        {
            ptx = ptx2 - pty2  + pos.x;
            pty = 2.0*tmpx*pty + pos.y;
        }

        length = sqrt(ptx2 + pty2);
        tlength += length;
        iter++;
    }
    while(length<maxlength && iter<maxiters);

    if(iter == maxiters)
    {
        FragmentColor = vec4(0,0,0,tlength/1000.0);
    }
    else FragmentColor = min(32.0, float(iter))/32.0 * vec4(glGetRed(tau), glGetGreen(tau), glGetBlue(tau), 1.0);
}


