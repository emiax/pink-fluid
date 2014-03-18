// Include standard headers
#include <stdio.h>
#include <stdlib.h>

//Include for a small timer
#include <ctime>
#include <cmath>

// Include GLEW
#ifdef __APPLE_CC__
#include <OpenGL/gl3.h>
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>


#define GLFW_INCLUDE_GL3
#define GLFW_NO_GLU
// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <iostream>

#include <common/Init.h>
#include <common/Shader.h>
#include <common/Texture2D.h>
#include <common/Texture3D.h>
#include <factories/levelSetFactories.h>
#include <ordinalGrid.h>
#include <state.h>
#include <simulator.h>
#include <velocityGrid.h>
#include <signedDistanceFunction.h>
#include <levelSet.h>
#include <stdlib.h>
#include <time.h>



int main( void ) {
  srand(time(NULL));

  //Create init object
  Init init = Init();

  //Initialize glfw
  init.glfw(4,1);
  //Open a window
  GLFWwindow* window = init.window(400, 400);

  //Print window info
  init.printWindowInfo(window);

  //Make opened window current context
  glfwMakeContextCurrent(window);

  init.glew();

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  glEnable(GL_CULL_FACE);


  // Nvidia cards require a vertex array to cooperate.
  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  //Set up the initial state.
  unsigned int w = 24, h = 24, d = 24;
  State prevState(w, h, d);
  State newState(w, h, d);

  VelocityGrid* velocities = new VelocityGrid(w, h, d);
  prevState.setVelocityGrid(velocities);



  /**
   * Init Level set object
   */
  LevelSet *ls = factory::levelSet::ball(w,h,d);
  prevState.setLevelSet(ls);
  newState.setLevelSet(ls);

  delete ls;

  // init simulator
  Simulator sim(&prevState, &newState, 0.1f);

  // Dark black background
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  //Load in shaders
  static ShaderProgram colorCubeProg("../vertShader.vert", "../colorCube.frag");
  static ShaderProgram rayCasterProg("../vertShader.vert", "../rayCaster.frag");

  static const GLfloat vertexBufferData[] = {
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f
  };

  static const GLuint triangleBufferData[] = {
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

  //Create us some buffers
  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData, GL_STATIC_DRAW);

  //Triangle coordinates
  glVertexAttribPointer(
                        0,                  // Location 0
                        3,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        0,                  // stride
                        (void*)0            // array buffer offset
                        );


  GLuint triangleBuffer;
  glGenBuffers(1, &triangleBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangleBufferData), triangleBufferData, GL_STATIC_DRAW);

  // Create framebuffer
  GLuint framebuffer;
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  GLuint backfaceTextureId;
  glGenTextures(1, &backfaceTextureId);
  glBindTexture(GL_TEXTURE_2D, backfaceTextureId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  GLuint volumeTextureId;
  glGenTextures(1, &volumeTextureId);
  glBindTexture(GL_TEXTURE_3D, volumeTextureId);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backfaceTextureId, 0);

  // fail check
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "failed to init FBO" << std::endl;
  } else {
    std::cout << "successfully initialized FBO" << std::endl;
  }


  //Object which encapsulates a texture + The destruction of a texture.
  Texture3D tex3D(w, h, d);
  double lastTime = glfwGetTime();
  int nbFrames = 0;

  float deltaT = 0.1; //First time step

  //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glfwSwapInterval(1);
  int i = 0;
  do{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); // bind the framebuffer

    //glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind the screen
    // common for both render passes.
    sim.step(deltaT);

    // deltaT = sim.getDeltaT();

    glm::mat4 matrix = glm::mat4(1.0f);
    matrix = glm::translate(matrix, glm::vec3(0.0f, 0.0f, 2.0f));
    matrix = glm::rotate(matrix, -3.1415926535f/2.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    // Render back face of the cube.
    colorCubeProg();
    glCullFace(GL_FRONT);

    {
      GLuint tLocation = glGetUniformLocation(colorCubeProg, "time");
      glUniform1f(tLocation, glfwGetTime());

      GLuint mvLocation = glGetUniformLocation(colorCubeProg, "mvMatrix");
      glUniformMatrix4fv(mvLocation, 1, false, glm::value_ptr(matrix));
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glEnableVertexAttribArray(0);
    glDrawElements(GL_TRIANGLES, 12*3, GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);



    // Do the ray casting.
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind the screen
    glCullFace(GL_BACK);
    rayCasterProg();


    {
      GLuint tLocation = glGetUniformLocation(rayCasterProg, "time");
      glUniform1f(tLocation, glfwGetTime());

      GLuint mvLocation = glGetUniformLocation(rayCasterProg, "mvMatrix");
      glUniformMatrix4fv(mvLocation, 1, false, glm::value_ptr(matrix));

      GLuint windowSizeLocation = glGetUniformLocation(rayCasterProg, "windowSize");
      glUniform2f(windowSizeLocation, width, height);
    }

    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, backfaceTextureId);
    GLuint textureLocation = glGetUniformLocation(rayCasterProg, "backfaceTexture");
    glUniform1i(textureLocation, 0);


    // Set the x,y positions in the texture, in order to visualize the velocity field.
    // Currently directly plots the mac-grid. Should perhaps use interpolation in order to use the
    // corresponding cell-value instead of the edge velocities.
    for(unsigned int k=0;k<d;++k) {
      for(unsigned int j = 0; j < h; ++j){
        for(unsigned int i=0;i<w;++i) {

          // velocity
          //tex3D.set(i,j,k,0, 0.5 + 0.5*newState.getVelocityGrid()->u->get(i,j,k));
          //tex3D.set(i,j,1, 0.5 + 0.5*newState.getVelocityGrid()->v->get(i,j));
          //tex3D.set(i,j,2, 0.5 + newState.getCellTypeGrid()->get(i, j));
          //tex3D.set(i,j,2, 0.5);
          //tex3D.set(i,j,3, 1.0f);

          // divergence
          //tex3D.set(i,j,0, fabs(sim.getDivergenceGrid()->get(i,j)));
          //tex3D.set(i,j,1, fabs(sim.getDivergenceGrid()->get(i,j)));
          //tex3D.set(i,j,2, fabs(sim.getDivergenceGrid()->get(i,j)));
          //tex3D.set(i,j,3, 1.0f);

          // type
          // tex3D.set(i,j,k, 0, newState.getCellTypeGrid()->get(i,j, k) == CellType::FLUID ? 1.0 : 0.0);
          // tex3D.set(i,j,k, 1, newState.getCellTypeGrid()->get(i,j, k) == CellType::FLUID ? 1.0 : 0.0);
          // tex3D.set(i,j,k, 2, newState.getCellTypeGrid()->get(i,j, k) == CellType::SOLID ? 1.0 : 0.0);
          // tex3D.set(i,j,k, 3, 1.0f);

          //signed dist
          float dist = newState.getSignedDistanceGrid()->get(i, j, k);
          float solid = newState.getCellTypeGrid()->get(i,j, k) == CellType::SOLID ? 1.0 : 0.0;
          dist = (glm::clamp(dist + solid, -1.0f, 1.0f)+1)/2;
          tex3D.set(i,j,k, 0, newState.getCellTypeGrid()->get(i,j, k) == CellType::SOLID ? 1.0 : 0.0);
          tex3D.set(i, j, k, 1, dist*0.3f);
          tex3D.set(i, j, k, 2, dist);
          tex3D.set(i, j, k, 3, 1.0f);

          //closest point
          // tex3D.set(i,j,0, newState.getClosestPointGrid()->get(i,j).x / 70.0);
          // tex3D.set(i,j,1, newState.getClosestPointGrid()->get(i,j).y / 70.0);
          // tex3D.set(i,j,2, 0.0f);
          // tex3D.set(i,j,3, 1.0f);
        }
      }
    }


    //    glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_3D, volumeTextureId);
    tex3D(GL_TEXTURE1);
    GLuint volumeTextureLocation = glGetUniformLocation(rayCasterProg, "volumeTexture");
    glUniform1i(volumeTextureLocation, 1);



    glEnableVertexAttribArray(0);
    glDrawElements(GL_TRIANGLES, 12*3, GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);

    //Get the uniformlocation of the texture from the shader.
    //    GLuint textureLocation = glGetUniformLocation(prog, "myFloatTex");
    //Use GL_TEXTURE0 as a textureposition for the texture.
    //tex2(GDgL_TEXTURE0);
    //Set the uniform so that the uniform maps 0 -> GL_TEXTURE0
    //    glUniform1i(textureLocation, 0);

    //glEnableVertexAttribArray(0);
    //    glEnableVertexAttribArray(1);

    //glDrawArrays(GL_TRIANGLE_STRIP, 0, 6); // 3 indices starting at 0 -> 1 triangle
    //    glDrawElements(GL_TRIANGLES, 12*3, GL_UNSIGNED_INT, 0);


    //    glDisableVertexAttribArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind the framebuffer
    glfwPollEvents();
    glfwSwapBuffers(window);
    double currentTime = glfwGetTime();
    nbFrames++;

    if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1 sec ago
      // printf and reset timer
      std::string title = std::to_string(1000.0/double(nbFrames)) + "ms/frame        " + std::to_string(deltaT) + "  dt";
      glfwSetWindowTitle(window, title.c_str());
      nbFrames = 0;
      lastTime += 1.0;
    }
    i++;
    // std::cin.get();
  } // Check if the ESC key was pressed or the window was closed
  while( !glfwWindowShouldClose(window) );
    std::cout << "Cleaning up!" << std::endl;
  // Close OpenGL window and terminate GLFW
  glfwDestroyWindow(window);
  glfwTerminate();
  glDeleteBuffers(1, &vertexbuffer);
  //  glDeleteBuffers(1, &uvbuffer);

  glDeleteVertexArrays(1, &VertexArrayID);
  exit(EXIT_SUCCESS);
}
//
