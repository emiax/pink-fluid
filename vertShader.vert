#version 400 core

layout(location = 0) in vec3 inputPosition;
uniform mat4 mvMatrix;
uniform float time;
out vec3 position;

void main(void) {
  vec4 column0 = vec4(1.0, 0.0, 0.0, 0.0);
  vec4 column1 = vec4(0.0, 1.0, 0.0, 0.0);
  vec4 column2 = vec4(0.0, 0.0, 1.0, 1.0);
  vec4 column3 = vec4(0.0, 0.0, 0.0, 0.0);
  mat4 pMatrix = mat4(column0, column1, column2, column3);
  
  gl_Position = pMatrix * mvMatrix * vec4(inputPosition, 1.0);
  position = inputPosition;
}
