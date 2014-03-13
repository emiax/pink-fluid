#version 400 core

uniform sampler2D backfaceTexture;
in vec3 position;
uniform vec2 windowSize;
uniform sampler3D volumeTexture;

out vec4 color;

void main() {
  vec3 frontCoord = 0.5*position+vec3(0.5, 0.5, 0.5);
  vec2 texCoords = vec2(gl_FragCoord.x / windowSize.x, gl_FragCoord.y / windowSize.y);
  vec3 backCoord = texture(backfaceTexture, texCoords).xyz;
  
  //  color = vec4(length(frontCoord - backCoord), 0.0, 0.0, 1.0);

  int gridSize = 20;
  vec3 step = normalize(frontCoord - backCoord)/(float(gridSize)); 

  int maxIter = gridSize * 2;
  
  float depth = length(frontCoord - backCoord);
  
  vec3 accumulated = vec3(0.0, 0.0, 0.0);
  for (int i = 0; i < maxIter; ++i) {
    // break if we are outside the volumeTexture.
    vec3 displacement = step*i;
    if (length(displacement) > depth) break;

    vec3 sampleCoord = backCoord + step*i;
    accumulated += texture(volumeTexture, sampleCoord).xyz/float(gridSize);
  }

  color = vec4(accumulated, 1.0);
}
