// Include GLEW
#ifdef __APPLE_CC__

#include <OpenGL/gl3.h>

#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif

#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>


#define GLFW_INCLUDE_GL3
#define GLFW_NO_GLU
// Include GLFW
#include <GLFW/glfw3.h>

#include <common/shader.h>
#include <glm/glm.hpp>




class Texture3D;
class FBO;
class State;

class RayCaster {
 public:
  RayCaster();
  ~RayCaster();
  void setResolution(int w, int h);
  void updateFrameBuffer();
  void updateVolume(State *state);
  void render(State* s, glm::mat4 mvp);
  void renderBubbles(State* state, glm::mat4 mvp);
  void renderFluid(State *s, glm::mat4 mvp);
 private:
  int windowWidth = 64;
  int windowHeight = 64;

  bool initialized;
  ShaderProgram *colorCubeProgram;
  ShaderProgram *rayCasterProgram;
  ShaderProgram *bubbleProgram;
  FBO *frameBuffer = nullptr;
  Texture3D *volumeTextureData;

  GLuint vertexBuffer;
  GLuint triangleBuffer;
  GLuint bubbleBuffer;
  GLuint volumeTexture;

  GLFWwindow *window;
  
  static constexpr GLfloat vertexBufferData[] = {
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f
  };
  
  static constexpr GLuint triangleBufferData[] = {
    // xy plane (z = -1)
    0, 1, 3,
    3, 2, 0,
    // xz plane (y = -1)
    0, 5, 1,
    0, 4, 5,
    // yz plane (x = -1)
    0, 2, 4,
    2, 6, 4,
    // xy plane (z = 1)
    4, 7, 5,
    4, 6, 7,
    // xz plane (y = 1)
    2, 7, 6,
    2, 3, 7,
    // yz plane (x = 1)
    1, 5, 3,
    3, 5, 7
  };
  
  std::vector<GLfloat> bubbleBufferData;
};
