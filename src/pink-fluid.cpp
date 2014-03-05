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

  // Dark blue background
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

  //Load in shaders
  static ShaderProgram prog("../vertShader.vert", "../fragShader.frag");

  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  static const GLfloat g_vertex_buffer_data[] = {
    -1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,

    1.0f, -1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f
  };

  static const GLfloat g_uv_buffer_data[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f
  };
  //Create us some buffers
  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

  GLuint uvbuffer;
  glGenBuffers(1, &uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

  //Set up the initial state.
  unsigned int w = 80, h = 80;
  State prevState(w, h);
  State newState(w, h);

  VelocityGrid* velocities = new VelocityGrid(w,h);
  prevState.setVelocityGrid(velocities);

  /**
   * Init Level set object
   */
  
  // define initial signed distance
  SignedDistanceFunction circleSD([&](const unsigned int &i, const unsigned int &j) {
      // distance function to circle with radius w/3, center in (w/2, h/2)
      const float x = (float)i - (float)w/3;
      const float y = (float)j - (float)h/2;
      return sqrt( x*x + y*y ) - (float)w/4;
    });

  Grid<CellType> *cellTypeGrid = new Grid<CellType>(w, h);
  // init boundary grid
  cellTypeGrid->setForEach([&](unsigned int i, unsigned int j){
    CellType bt = CellType::EMPTY;
    if(i == 0){
      bt = CellType::SOLID;
    }
    else if(j == 0){
      bt = CellType::SOLID;
    }
    else if(i == w - 1){
      bt = CellType::SOLID;
    }
    else if(j == h - 1){
      bt = CellType::SOLID;
    }
    return bt;
  });

  LevelSet *ls = new LevelSet( w, h, circleSD, cellTypeGrid );
  prevState.setLevelSet(ls);
  newState.setLevelSet(ls);

  // init simulator
  Simulator sim(&prevState, &newState,0.1f);

  //Object which encapsulates a texture + The destruction of a texture.
  Texture2D tex2D(w, h);
  double lastTime = glfwGetTime();
  int nbFrames = 0;

  float deltaT = 0.1; //First time step

  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glfwSwapInterval(1);
  do{

    sim.step(deltaT);
    deltaT = sim.getDeltaT();
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    prog();

    GLuint location = glGetUniformLocation(prog, "time");
    glUniform1f(location, glfwGetTime());

    // Set the x,y positions in the texture, in order to visualize the velocity field.
    // Currently directly plots the mac-grid. Should perhaps use interpolation in order to use the
    // corresponding cell-value instead of the edge velocities.
    for(unsigned int j = 0; j < h; ++j){
      for(unsigned int i=0;i<w;++i) {

        // velocity
        //tex2D.set(i,j,0, 0.5 + 0.5*newState.getVelocityGrid()->u->get(i,j));
        //tex2D.set(i,j,1, 0.5 + 0.5*newState.getVelocityGrid()->v->get(i,j));
        //tex2D.set(i,j,2, 0.5 + newState.getCellTypeGrid()->get(i, j));
        //tex2D.set(i,j,2, 0.5);
        //tex2D.set(i,j,3, 1.0f);

        // divergence
        //tex2D.set(i,j,0, fabs(sim.getDivergenceGrid()->get(i,j)));
        //tex2D.set(i,j,1, fabs(sim.getDivergenceGrid()->get(i,j)));
        //tex2D.set(i,j,2, fabs(sim.getDivergenceGrid()->get(i,j)));
        //tex2D.set(i,j,3, 1.0f);

        // type
        // tex2D.set(i,j,0, newState.getCellTypeGrid()->get(i,j) == CellType::EMPTY ? 1.0 : 0.0);
        // tex2D.set(i,j,1, newState.getCellTypeGrid()->get(i,j) == CellType::SOLID ? 1.0 : 0.0);
        // tex2D.set(i,j,2, newState.getCellTypeGrid()->get(i,j) == CellType::FLUID ? 1.0 : 0.0);
        // tex2D.set(i,j,3, 1.0f);


        //signed dist
        tex2D.set(i,j,0, newState.getSignedDistanceGrid()->get(i,j));
        tex2D.set(i,j,1, newState.getSignedDistanceGrid()->get(i,j));
        tex2D.set(i,j,2, 1.0f);
        tex2D.set(i,j,3, 1.0f);

        //closest point
        /*tex2D.set(i,j,0, newState.getClosestPointGrid()->get(i,j).x / 70.0);
        tex2D.set(i,j,1, newState.getClosestPointGrid()->get(i,j).y / 70.0);
        tex2D.set(i,j,2, 0.0f);
        tex2D.set(i,j,3, 1.0f);*/

      }
    }

    //Get the uniformlocation of the texture from the shader.
    GLuint textureLocation = glGetUniformLocation(prog, "myFloatTex");
    //Use GL_TEXTURE0 as a textureposition for the texture.
    tex2D(GL_TEXTURE0);
    //Set the uniform so that the uniform maps 0 -> GL_TEXTURE0
    glUniform1i(textureLocation, 0);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    //Triangle coordinates
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
                          0,                  // Location 0
                          3,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    //UV Texture coordinates
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
                          1,                  //Location 1
                          2,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    // Draw the triangle !

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6); // 3 indices starting at 0 -> 1 triangle

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

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

    // std::cin.get();
  } // Check if the ESC key was pressed or the window was closed
  while( !glfwWindowShouldClose(window) );
    std::cout << "Cleaning up!" << std::endl;
  // Close OpenGL window and terminate GLFW
  glfwDestroyWindow(window);
  glfwTerminate();
  glDeleteBuffers(1, &vertexbuffer);
  glDeleteBuffers(1, &uvbuffer);
  glDeleteVertexArrays(1, &VertexArrayID);
  exit(EXIT_SUCCESS);
}
