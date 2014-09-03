#version 400 core

layout(location = 0) in vec4 inputData;
out float radius;
out vec2 pos;


void main(void) {
  gl_Position = vec4(inputData.x*2.0 - 1.0, -(inputData.y * 2.0 - 1.0), 0.0, 1.0);
  gl_PointSize = inputData.w*20.00;
  radius = inputData.w;
  pos = vec2(inputData.x*2.0 - 1.0, -(inputData.y * 2.0 - 1.0));
}
