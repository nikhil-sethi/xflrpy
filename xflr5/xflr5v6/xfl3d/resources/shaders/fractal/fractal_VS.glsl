#version 410 core

uniform vec2  ViewTrans;
uniform float ViewScale;
uniform float ViewRatio;

layout(location = 0) in vec2 VertexPosition;

out float posx;
out float posy;

void main(void)
{
    posx = (VertexPosition.x/ViewScale*ViewRatio + ViewTrans.x);
    posy = (VertexPosition.y/ViewScale           + ViewTrans.y);
    gl_Position = vec4(VertexPosition.x, VertexPosition.y, 0.0f, 1.0f);
}
