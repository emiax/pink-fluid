// Include standard headers
#include <stdio.h>
#include <stdlib.h>


//Include for a small timer
#include <ctime>

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
  //Create new velocity positions
<<<<<<< HEAD
  for(unsigned int i = 3; i < w/3; i++){
    for(unsigned int j = 3; j < h/3; j++){
=======
    for(unsigned int i = 2; i < w/3; i++){
    for(unsigned int j = 2; j < h/3; j++){
>>>>>>> 29ec12c6ac22212cf191cb6ed8217ace97eac02f
      velocities->u->set( i, j, 1.0f );
    }
  }
  for(unsigned int i = 3; i < w/3; i++){
    for(unsigned int j = 3; j < h/3; j++){
      velocities->v->set( i, j, -1.0f );
    }
  }

  for(unsigned int i = 2*w/3; i < w-3; i++){
    for(unsigned int j = 2*h/3; j < h-3; j++){
      velocities->u->set( i, j, -1.0f );
    }
  }
  for(unsigned int i = 2*w/3; i < w-3; i++){
    for(unsigned int j = 2*h/3; j < h-3; j++){
      velocities->v->set( i, j, 1.0f );
    }
    }

  prevState.setVelocityGrid(velocities);

  
  Grid<BoundaryType> *boundaries = new Grid<BoundaryType>(w, h);
  // init boundary grid
  for(unsigned int i = 0; i < w; i++){
    for(unsigned int j = 0; j < h; j++){
      BoundaryType bt;
      if(i < 3){
        bt = BoundaryType::SOLID;
      }
      else if(j < 3){
        bt = BoundaryType::SOLID;
      }
      else if(i > w - 3){
        bt = BoundaryType::SOLID;
      }
      else if(j > h - 3){
        bt = BoundaryType::SOLID;
      }
      else if (j > h/3 && j < 2*h/3 && i > 2 && i < w-2) {
        bt = BoundaryType::FLUID;
      } else {
        bt = BoundaryType::FLUID;
      }
      boundaries->set(i,j, bt);
    }
  }
  prevState.setBoundaryGrid(boundaries);

  // instantiate ink grid
  OrdinalGrid<glm::vec3> *ink = new OrdinalGrid<glm::vec3>(w, h);
  for (unsigned int j = 0; j < h; ++j) {
    for (unsigned int i = 0; i < w; ++i) {
      ink->set( i, j, glm::vec3(0.0f) );
    }
  }
<<<<<<< HEAD
  for(unsigned int i = 3; i < 25; i++){
    for(unsigned int j = 3; j < 25; j++){
      ink->set( i, j, glm::vec3(1, 0, 1) );
    }
  }
  for(unsigned int i = 2*w/3; i < w-3; i++){
    for(unsigned int j = 2*h/3; j < h-3; j++){
      ink->set( i, j, glm::vec3(0, 1, 1) );
=======
  for(unsigned int i = 1; i < w/3; i++){
    for(unsigned int j = 1; j < h/3; j++){
      ink->set( i, j, glm::vec3(0.5, 0, 0.5) );
    }
  }
  for(unsigned int i = 2*w/3; i < w-1; i++){
    for(unsigned int j = 2*h/3; j < h-1; j++){
      ink->set( i, j, glm::vec3(0, 0.5, 0.5) );
>>>>>>> 29ec12c6ac22212cf191cb6ed8217ace97eac02f
    }
  }
  prevState.setInkGrid(ink);
  
  // init simulator
  Simulator sim(&prevState, &newState,0.1f);

  //Object which encapsulates a texture + The destruction of a texture.
  Texture2D tex2D(w, h);
  double lastTime = glfwGetTime();
  int nbFrames = 0;

  float deltaT = 0.1; //First time step


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // float lastRun = glfwGetTime();
  glfwSwapInterval(1);
  do{
    // lastRun = glfwGetTime();
    // float deltaT = glfwGetTime()-lastRun;
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
          

          /*       tex2D.set(i,j,0, newState.getVelocityGrid()->u->get(i,j));
          tex2D.set(i,j,1, newState.getVelocityGrid()->v->get(i,j));
          tex2D.set(i,j,2, newState.getBoundaryGrid()->get(i, j));
          tex2D.set(i,j,3, 1.0f);*/

          //tex2D.set(i,j,0, newState.getInkGrid()->get(i,j).x);
          //          tex2D.set(i,j,1, newState.getInkGrid()->get(i,j).y);
          tex2D.set(i,j,0, newState.getBoundaryGrid()->get(i,j) == BoundaryType::FLUID ? 1.0 : 0.0);
          tex2D.set(i,j,1, newState.getBoundaryGrid()->get(i,j) == BoundaryType::FLUID ? 1.0 : 0.0);
          tex2D.set(i,j,2, newState.getBoundaryGrid()->get(i,j) == BoundaryType::SOLID ? 1.0 : 0.0);
          tex2D.set(i,j,3, 1.0f);

          tex2D.set(i,j,0, 0.5 + 0.5*newState.getVelocityGrid()->u->get(i,j));
          tex2D.set(i,j,1, 0.5 + 0.5*newState.getVelocityGrid()->v->get(i,j));
          //          tex2D.set(i,j,2, 0.5 + newState.getBoundaryGrid()->get(i, j))
          tex2D.set(i,j,2, 0.5);
          tex2D.set(i,j,3, 1.0f);

          tex2D.set(i,j,0, newState.getInkGrid()->get(i,j).x);
          tex2D.set(i,j,1, newState.getInkGrid()->get(i,j).y);
          tex2D.set(i,j,2, newState.getInkGrid()->get(i,j).z);
          tex2D.set(i,j,3, 1.0f);

          //tex2D.set(i,j,0, fabs(sim.getDivergenceGrid()->get(i,j)));
          //tex2D.set(i,j,1, fabs(sim.getDivergenceGrid()->get(i,j)));
          //tex2D.set(i,j,2, fabs(sim.getDivergenceGrid()->get(i,j)));
          //tex2D.set(i,j,3, 1.0f);

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
