// Include GLEW
#ifdef __APPLE_CC__
#include <OpenGL/gl3.h>
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif

#include <FBO.h>
#include <common/Shader.h>


/**
 * GPU-accelerated pressure solver
 */
class PressureSolver {
public:
  PressureSolver(unsigned int w, unsigned int h) {
    fboFrom = new FBO(w, h);
    fboTo = new FBO(w, h);
    prog = new ShaderProgram("../vertShader.vert", "../pressureShader.frag");
  };

  ~PressureSolver() {
    delete fboFrom;
    delete fboTo;
  };

  Texture2D const *const solve(unsigned int iterations) {

    // clear textures
    (*prog)();

    for(unsigned i = 0; i < iterations; ++i) {
      fboFrom->activateRead();
      fboTo->activateWrite();

      GLuint textureLocation = glGetUniformLocation((*prog), "pressureTex");
      glUniform1i(textureLocation, fboFrom->getTextureId());

      swapFrameBuffers();
    }

    FBO::deactivateFramebuffers();
    return fboFrom->getTexture();
  }

private:
  FBO *fboFrom, *fboTo;

  ShaderProgram *prog;

  void swapFrameBuffers() {
    FBO *tmp;
    tmp = fboFrom;
    fboFrom = fboTo;
    fboTo = tmp;
  };
};