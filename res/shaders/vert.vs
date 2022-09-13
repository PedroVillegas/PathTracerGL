#version 410 core

layout(location = 0) in vec4 aPos;
layout(location = 1) in vec4 aCol;

out vec4 vertCol;

void main()
{
    gl_Position = aPos;
    vertCol = aCol;
}
