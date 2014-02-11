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
#include <OrdinalGrid.h>
#include <state.h>
#include <simulator.h>

int main( void ) {
  
  //Create init object
  Init init = Init();

  //Initialize glfw
  init.glfw(4,1);
  glfwSwapInterval(1);
  //Open a window
  GLFWwindow* window = init.window();

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
    1.0f,-1.0f, 0.0f,

    1.0f,-1.0f, 0.0f,
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

  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

  GLuint uvbuffer;
  glGenBuffers(1, &uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);


  int w = 100, h = 100;
  State prevState(w, h);
  OrdinalGrid<float>** velocities = new OrdinalGrid<float>*[2];
  std::cout << "Setting up basic state" << std::endl;
  velocities[0] = new OrdinalGrid<float>(w+1, h);
  velocities[1] = new OrdinalGrid<float>(w, h+1);
  for(int i = 0; i <= w; i++){
    for(int j = 0; j < h; j++){
      velocities[0]->set(i,j,i*0.1);
    }
    
  }
  for(int i = 0; i < w; i++){
    for(int j = 0; j <= h; j++){
      velocities[1]->set(i,j,0);
    }
  }
  prevState.setVelocityGrid(velocities);
  State newState(w, h);
  State tempState(w, h);
  Simulator sim(w, h);
  
  Texture2D tex2D(w, h);

  float lastRun = glfwGetTime();
  do{
    lastRun = glfwGetTime();
    float deltaT = glfwGetTime()-lastRun;
    sim.advect(&prevState, &newState, 1);
     
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    prog();


    GLuint location = glGetUniformLocation(prog, "time");
    glUniform1f(location, glfwGetTime());

    for(int i=0;i<w;++i) {
      for(int j = 0; j < h; ++j){
        tex2D.set(i,j,0, newState.getVelocityGrid()[0]->get(i,j));
        tex2D.set(i,j,1, newState.getVelocityGrid()[1]->get(i,j));
        tex2D.set(i,j,2, 0.0f);
        tex2D.set(i,j,3, 1.0f);
      }
    }

    GLuint textureLocation = glGetUniformLocation(prog, "myFloatTex");
    tex2D(GL_TEXTURE0);
    glUniform1i(textureLocation, 0);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
                          0,                  // Location 0
                          3,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    

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

    tempState = prevState;
    prevState = newState;
    newState = tempState;
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
