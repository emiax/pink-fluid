#version 400 core

in float radius;
in vec2 pos;
out vec4 color;
uniform float windowWidth;
uniform float windowHeight;
uniform float w;
uniform float h;

vec2 pos2Screen(vec2 pos) {
  return vec2(windowWidth * pos.x / w, windowHeight * (1.0-(pos.y / h)));
}

void main(void) {
  float dist = distance(gl_FragCoord.xy, pos2Screen(pos));

  float a = step(dist, abs(radius)*windowWidth/w/2.0);
  float b = 1.0 - step(dist, abs(radius)*windowWidth/w/2.0 - 1.0);

  color = vec4(0.9, 0.9, 0.9, a*0.4);
}    
