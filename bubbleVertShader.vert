#version 400 core

layout(location = 0) in vec4 inputData;
out float radius;
out vec2 pos;

uniform float w;
uniform float windowWidth;
uniform float h;
uniform float windowHeight;


void main(void) {
  gl_Position = vec4(inputData.x/w*2.0 - 1.0, -(inputData.y/h * 2.0 - 1.0), 0.0, 1.0);
  gl_PointSize = abs(inputData.w) * windowWidth / w;
  radius = inputData.w;
  pos = vec2(inputData.x, inputData.y);
}
