// Include GLEW
#ifdef __APPLE_CC__
#include <OpenGL/gl3.h>
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif

#include <FBO.h>
#include <common/Shader.h>
#include <ordinalGrid.h>

/**
 * GPU-accelerated pressure solver
 */
class PressureSolver {
public:
  PressureSolver(unsigned int w, unsigned int h) {
    this->w = w;
    this->h = h;
    fboFrom = new FBO(w, h, GL_TEXTURE1);
    fboTo = new FBO(w, h, GL_TEXTURE2);
    prog = new ShaderProgram("../vertShader.vert", "../pressureShader.frag");
    divergenceTex = new Texture2D(w, h);
    GLuint textureLocation = glGetUniformLocation((*prog), "divergenceTex");
    glUniform1i(textureLocation, divergenceTex->getId());
  };

  ~PressureSolver() {
    delete fboFrom;
    delete fboTo;
  };

  Texture2D const *const solve(unsigned int iterations, OrdinalGrid<float> const* divergenceGrid) {

    divergenceGridToTexture(divergenceGrid);

    // clear textures
    (*prog)();

    for(unsigned i = 0; i < iterations; ++i) {
      fboTo->activateWrite();

      GLuint textureLocation = glGetUniformLocation((*prog), "pressureTex");
      glUniform1i(textureLocation, fboFrom->getTextureId());

      swapFrameBuffers();
    }

    FBO::deactivateFramebuffers();
    return fboTo->getTexture();
  }

private:
  unsigned int w, h;
  FBO *fboFrom, *fboTo;
  Texture2D *divergenceTex;

  ShaderProgram *prog;

  void swapFrameBuffers() {
    FBO *tmp;
    tmp = fboFrom;
    fboFrom = fboTo;
    fboTo = tmp;
  };

  void divergenceGridToTexture(OrdinalGrid<float> const* divergenceGrid) {
    for(unsigned j = 0; j < h; ++j) {
      for(unsigned i = 0; i < w; ++i) {
        divergenceTex->set(i, j, 0, divergenceGrid->get(i, j));
      }
    }
    (*divergenceTex)(GL_TEXTURE3);
  };
};