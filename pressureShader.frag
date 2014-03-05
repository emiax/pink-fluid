#version 400 core

in vec2 fragPosition;
uniform sampler2D pressureTex;

// Ouput data
out vec4 color;
void main()
{
  vec4 value = texture(pressureTex, fragPosition);

    
  
  color = value;
}
