#version 400 core

in vec2 fragPosition;
uniform sampler2D myFloatTex;
// Output data
out vec4 color;

void main()
{
  vec4 value = texture(myFloatTex, fragPosition);
  color = value;
}
