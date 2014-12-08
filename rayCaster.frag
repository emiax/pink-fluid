#version 400 core

uniform sampler2D backfaceTexture;
in vec3 position;
uniform vec2 windowSize;
uniform sampler3D volumeTexture;
uniform mat4 mvMatrix;

out vec4 color;
#define THRESHOLD 0.000001
void main() {
  vec3 frontCoord = 0.5*position+vec3(0.5); // world coord -> tex coord
  vec2 texCoords = vec2(gl_FragCoord.x / windowSize.x, gl_FragCoord.y / windowSize.y);
  vec3 backCoord = texture(backfaceTexture, texCoords).xyz;

  //int gridSize = 16;

  ivec3 gridSizeVec = textureSize(volumeTexture, 0);
  int gridSize = gridSizeVec.x;
  
  float samplesPerCell = 2.0;
  //vec3 step = normalize(backCoord - frontCoord)/(float(gridSize)*samplesPerCell);


  vec3 castDirection = normalize(backCoord - frontCoord);
  vec3 displacement = vec3(0.0);
  
  int maxIter = int(length(vec3(gridSize))) * int(samplesPerCell);

  float depth = length(frontCoord - backCoord);

  color = vec4(0.0, 0.0, 0.0, 0.0);

  float cellSize = 1.0/gridSize;
  float offset = cellSize;

  vec3 accumulated = vec3(0.0, 0.0, 0.0);

  float iterations = 0.0;

  for (int i = 0; i < maxIter; ++i) {

    //    vec3 displacement = step * i;
    // break if we are outside the volumeTexture.
    if (length(displacement) > depth) break;

    vec3 mid = frontCoord + displacement;
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

    vec3 bGradient = normalize(vec3(dvdx.b, dvdy.b, dvdz.b));
    vec3 light = normalize(vec3(1.0, 1.0, 1.0));
    float diffuseCoefficient = dot(light, bGradient);

    
    // hitting the water interface?
    if (b - 0.05 < 0) {
      color = vec4(0.3, diffuseCoefficient, 0.8, 0.5);
      break;
    }

    // hitting a wall?
    //    if (solid > 0.5){
    // color += vec4(0.2,0.2,0.2,0.2);
    //}

    // minimal distance to surface is the value of the level set.
    float minDistanceToSurface = min(b/gridSize, 1.0);
    iterations += 1.0;
    
    displacement += castDirection*minDistanceToSurface;
    
    //color = vec4(bGradient, 1.0);
  }

  //color = vec4(vec3(iterations)/10, 1.0);

  //  color = vec4(accumulated, 1.0);
}
