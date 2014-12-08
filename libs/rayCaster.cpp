#include <rayCaster.h>
#include <state.h>
#include <common/texture3D.h>
#include <common/FBO.h>
#include <common/Init.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ordinalGrid.h>

constexpr GLfloat RayCaster::vertexBufferData[];
constexpr GLuint RayCaster::triangleBufferData[];


RayCaster::RayCaster() {
  Init init = Init();

  //Initialize glfw
  init.glfw(4, 1);
  //Open a window
  window = init.window(400, 400);

  //Print window info
  init.printWindowInfo(window);

  //Make opened window current context
  glfwMakeContextCurrent(window);

  init.glew();

  glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

  glViewport(0, 0, windowWidth, windowHeight);

  glEnable(GL_CULL_FACE);

  // Nvidia cards require a vertex array to cooperate.
  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData, GL_STATIC_DRAW);

  glGenBuffers(1, &triangleBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangleBufferData), triangleBufferData, GL_STATIC_DRAW);

  glGenBuffers(1, &bubbleBuffer);

  glGenTextures(1, &volumeTexture);
  glBindTexture(GL_TEXTURE_3D, volumeTexture);

  volumeTextureData = nullptr;

  // Dark black background
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  colorCubeProgram = new ShaderProgram("../vertShader.vert", "../colorCube.frag");
  rayCasterProgram = new ShaderProgram("../vertShader.vert", "../rayCaster.frag");
  bubbleProgram = new ShaderProgram("../bubbleVertShader.vert", "../bubbleFragShader.frag");
}

RayCaster::~RayCaster() {
  delete colorCubeProgram;
  delete rayCasterProgram;
  delete bubbleProgram;
}

void RayCaster::updateFrameBuffer() {
  if (frameBuffer == nullptr || frameBuffer->getWidth() != windowWidth || frameBuffer->getHeight() != windowHeight) {
    if (frameBuffer != nullptr) {
      delete frameBuffer;
    }
    frameBuffer = new FBO(windowWidth, windowHeight);
  }
}


void RayCaster::updateVolume(State* state) {
  if (volumeTextureData == nullptr ||
      state->getW() != volumeTextureData->getWidth() ||
      state->getH() != volumeTextureData->getHeight() ||
      state->getD() != volumeTextureData->getDepth()) {
    if (volumeTextureData != nullptr) {
      delete volumeTextureData;
    }
    volumeTextureData = new Texture3D(state->getW(), state->getH(), state->getD());
  }
}

void RayCaster::renderFluid(State* state, glm::mat4 mvp) {
  (*colorCubeProgram)();
  glCullFace(GL_FRONT);

  frameBuffer->activate();
  GLuint mvpLocation = glGetUniformLocation(*colorCubeProgram, "mvMatrix");
  glUniformMatrix4fv(mvpLocation, 1, false, glm::value_ptr(mvp));

  glClear(GL_COLOR_BUFFER_BIT);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleBuffer);
  //Triangle coordinates
  glVertexAttribPointer(
                        0,                  // Location 0
                        3,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        0,                  // stride
                        (void *) 0            // array buffer offset
                        );

  glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, 0);
  glDisableVertexAttribArray(0);

  // Do the ray casting.
  glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind the screen
  glCullFace(GL_BACK);
  (*rayCasterProgram)();

  GLuint mvLocation = glGetUniformLocation(*rayCasterProgram, "mvMatrix");
  glUniformMatrix4fv(mvLocation, 1, false, glm::value_ptr(mvp));

  GLuint windowSizeLocation = glGetUniformLocation(*rayCasterProgram, "windowSize");
  glUniform2f(windowSizeLocation, windowWidth, windowHeight);


  glClear(GL_COLOR_BUFFER_BIT);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, *(frameBuffer->getTexture()));

  GLuint textureLocation = glGetUniformLocation(*rayCasterProgram, "backfaceTexture");
  glUniform1i(textureLocation, 0);

  const OrdinalGrid<float> *sdf = state->getSignedDistanceGrid();
  for (unsigned int k = 0; k < sdf->getD(); ++k) {
    for (unsigned int j = 0; j < sdf->getH(); ++j) {
      for (unsigned int i = 0; i < sdf->getW(); ++i) {
        float dist = sdf->get(i, j, k);
        float solid = state->getCellTypeGrid()->get(i, j, k) == CellType::SOLID ? 1.0f : 0.0f;
        volumeTextureData->set(i, j, k, 0, solid);
        volumeTextureData->set(i, j, k, 1, 0.0f); // not used
        volumeTextureData->set(i, j, k, 2, dist);
        volumeTextureData->set(i, j, k, 3, 1.0f);
      }
    }
  }

  (*volumeTextureData)(GL_TEXTURE1);
  GLuint volumeTextureLocation = glGetUniformLocation(*rayCasterProgram, "volumeTexture");
  glUniform1i(volumeTextureLocation, 1);

  glEnableVertexAttribArray(0);
  glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, 0);
  glDisableVertexAttribArray(0);

  FBO::deactivate();

}



void RayCaster::renderBubbles(State* state, glm::mat4 mvp) {
  const std::vector<Bubble> bubbles = state->getBubbles();
  bubbleBufferData.clear();
  //  std::cout << "frame=" << i << ", nBubbles=" << bubbles.size() << std::endl;

  int w = state->getW();
  int h = state->getH();
  int d = state->getD();

  for (int i = 0; i < bubbles.size(); i++) {
    Bubble b = bubbles.at(i);

    bubbleBufferData.push_back(b.position.x / (float)w * 2.0 - 1.0);
    bubbleBufferData.push_back(b.position.y / (float)h * 2.0 - 1.0);
    bubbleBufferData.push_back(b.position.z / (float)d * 2.0 - 1.0);
    bubbleBufferData.push_back(b.radius);
  }

  glBindBuffer(GL_ARRAY_BUFFER, bubbleBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bubbleBufferData.size(), bubbleBufferData.data(), GL_DYNAMIC_DRAW);

  (*bubbleProgram)();
  glEnable(GL_PROGRAM_POINT_SIZE);

  GLuint mvLocation = glGetUniformLocation(*bubbleProgram, "mvMatrix");
  glUniformMatrix4fv(mvLocation, 1, false, glm::value_ptr(mvp));

  glPointSize(4.0);

  if (bubbleBufferData.size() > 0) {
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
                          0,                  //Location 0
                          4,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glDrawArrays(GL_POINTS, 0, 4 * bubbleBufferData.size());
    glDisableVertexAttribArray(0);
    glDisable(GL_PROGRAM_POINT_SIZE);
  }
}

void RayCaster::render(State* state, glm::mat4 mvp) {
  updateFrameBuffer();
  updateVolume(state);


  renderFluid(state, mvp);
  renderBubbles(state, mvp);


  glfwPollEvents();
  glfwSwapBuffers(window);
}

void RayCaster::setResolution(int w, int h) {
  windowWidth = w;
  windowHeight = h;
}
