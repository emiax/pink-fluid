#version 400 core

in vec3 position;
out vec4 color;

void main()
{
  color = vec4(0.5*position+vec3(0.5, 0.5, 0.5), 1.0);
}
