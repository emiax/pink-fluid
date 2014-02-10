#version 400 core

layout(location = 0) in vec3 inputPosition;
layout(location = 1) in vec2 texCoord;
uniform float time;

out vec2 fragPosition;
void main(void) {
  fragPosition = texCoord;
  gl_Position = vec4(inputPosition,1);
}
