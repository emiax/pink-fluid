#version 400 core

uniform sampler2D backfaceTexture;
in vec3 position;
uniform vec2 windowSize;
uniform sampler3D volumeTexture;

out vec4 color;
#define THRESHOLD 0.000001
void main() {
  vec3 frontCoord = 0.5*position+vec3(0.5, 0.5, 0.5);
  vec2 texCoords = vec2(gl_FragCoord.x / windowSize.x, gl_FragCoord.y / windowSize.y);
  vec3 backCoord = texture(backfaceTexture, texCoords).xyz;

  //  color = vec4(length(frontCoord - backCoord), 0.0, 0.0, 1.0);

  int gridSize = 48;
  float samplesPerCell = 2.0;
  vec3 step = normalize(backCoord - frontCoord)/(float(gridSize)*samplesPerCell);

  int maxIter = gridSize * int(samplesPerCell);

  float depth = length(frontCoord - backCoord);

  color = vec4(0.0, 0.0, 0.0, 0.0);

  float cellSize = 1.0/gridSize;
  float offset = cellSize/5.0;

  vec3 accumulated = vec3(0.0, 0.0, 0.0);
  for (int i = 0; i < maxIter; ++i) {
    // break if we are outside the volumeTexture.
    vec3 displacement = step*i;
    if (length(displacement) > depth) break;

    vec3 mid = frontCoord + step*i;
    vec3 left = vec3(mid.x - offset, mid.yz);
    vec3 right = vec3(mid.x + offset, mid.yz);
    vec3 up = vec3(mid.x, mid.y - offset, mid.z);
    vec3 down = vec3(mid.x, mid.y + offset, mid.z);
    vec3 front = vec3(mid.xy, mid.z - offset);
    vec3 back = vec3(mid.xy, mid.z + offset);

    vec4 current = texture(volumeTexture, mid);
    vec4 dvdx = texture(volumeTexture, right) - texture(volumeTexture, left);
    vec4 dvdy = texture(volumeTexture, down) - texture(volumeTexture, up);
    vec4 dvdz = texture(volumeTexture, back) - texture(volumeTexture, front);

    float b = current.z;
    float solid = current.x;
    vec3 bGradient = vec3(dvdx.b, dvdy.b, dvdz.b);
    if (solid < 0.5){
      color += vec4(0.2,0.2,0.2,0.2);
    }
    if (abs(b - 0.5) < 0.05) {
      color = vec4(0.3, 0.3, 0.8, 0.5);
      break;
    }

  }

  //  color = vec4(accumulated, 1.0);
}
