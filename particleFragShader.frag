#version 400 core

in float radius;
in vec2 pos;
out vec4 color;

void main(void) {   
  color = vec4(radius, 0.0, 0.0, 1.0 /*step(radius, distance(gl_FragCoord.xy, pos)*/);
}    
