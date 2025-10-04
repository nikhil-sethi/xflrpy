#version 410 core

uniform int maxiters;
uniform float maxlength;

in float posx;
in float posy;

layout(location = 0) out vec4 FragmentColor;


void main(void)
{
//    The Mandelbrot set is the set of complex numbers c for which the function f_c(z) = z²+c
//    does not diverge when iterated from z=0
    double ptx, pty, ptx2, pty2;
    double tmpx;
    int iter=0;
    do
    {
        ptx2 = ptx*ptx;
        pty2 = pty*pty;
        tmpx = ptx;
        ptx = ptx2 - pty2  + double(posx);
        pty = 2.0*tmpx*pty + double(posy);
        iter++;
    }
    while(ptx2 + pty2<maxlength*maxlength && iter<maxiters);

    if(iter == maxiters) FragmentColor = vec4(0,0,0,1);
    else                 FragmentColor = vec4(float(iter)/float(maxiters), 0, 0, 1);
}
