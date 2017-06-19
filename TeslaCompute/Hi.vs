#version 330
layout (location = 0) in vec3 vposition;
void main() 
{
gl_Position = vec4(vposition, 1.0);
}