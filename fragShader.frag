#version 400 core

in vec2 fragPosition;
uniform sampler2D myFloatTex;
in vec3 position;
// Ouput data
out vec4 color;

void main()
{
  vec4 value = texture(myFloatTex, fragPosition);
  color = value;
  color = vec4(0.5*position+vec3(0.5, 0.5, 0.5), 1.0);
  //color = vec4(0.6, 0.0, 0.0, 1.0);
}
