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

#include <OrdinalGrid.h>
#include <state.h>
#include <simulator.h>

int main( void ) {
  
  //Create init object
  Init init = Init();

  //Initialize glfw
  init.glfw(4,1);

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
    0.0f, 1.0f
  };

  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

  GLuint uvbuffer;
  glGenBuffers(1, &uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);


  int w = 250, h = 250;
  int d = 4;
  State prevState(w, h);
  OrdinalGrid<float>** velocities = new OrdinalGrid<float>*[2];
  std::cout << "Setting up basic state" << std::endl;
  velocities[0] = new OrdinalGrid<float>(w+1, h);
  velocities[1] = new OrdinalGrid<float>(w, h+1);
  for(int i = 0; i <= w; i++){
    for(int j = 0; j < h; j++){
      velocities[0]->set(i,j,0.5f);
    }
  }
  for(int i = 0; i < w; i++){
    for(int j = 0; j <= h; j++){
      velocities[1]->set(i,j,0);
    }
  }
  prevState.setVelocityGrid(velocities);
  State newState(w, h);
  State tempState(w,h);
  Simulator sim(w,h);

  GLuint textureID;
  glGenTextures(1, &textureID);
  
  glBindTexture(GL_TEXTURE_2D, textureID);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);	
  std::cout << "Generated texture" << std::endl;
  std::cout << "Created texture" << std::endl;
  GLfloat *data = new GLfloat[h*w*d];
  for (int i = 0; i < h; ++i) {
    for (int j = 0; j < w; ++j){
      data[i*w*d + j*d + 0] = 0.0f;
      data[i*w*d + j*d + 1] = 0.0f;
      data[i*w*d + j*d + 2] = 0.0f;
      data[i*w*d + j*d + 3] = 1.0f;
    }
  }
  std::cout << "Wrote to texture data" << std::endl;
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  std::cout << "Everything ready for first render pass" << std::endl;
  float lastRun = glfwGetTime();
  do{
    float deltaT = glfwGetTime()-lastRun;
    lastRun = glfwGetTime();
    sim.step(&prevState, &newState,deltaT);
     
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    prog();


    GLuint location = glGetUniformLocation(prog, "time");
    glUniform1f(location, glfwGetTime());
    GLuint textureLocation = glGetUniformLocation(prog, "myFloatTex");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(textureLocation, 0);
    for(int i=0;i<w;++i) {
      for(int j = 0; j < h; ++j){
        data[i*w*d + j*d + 0] = newState.getVelocityGrid()[0]->get(i,j);
        data[i*w*d + j*d + 1] = newState.getVelocityGrid()[1]->get(i,j);
        data[i*w*d + j*d + 2] = 0.0f;
        data[i*w*d + j*d + 3] = 1.0f;
      }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, data);


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

  // Close OpenGL window and terminate GLFW
  glfwDestroyWindow(window);
  glfwTerminate();
  glDeleteBuffers(1, &vertexbuffer);
  glDeleteBuffers(1, &uvbuffer);
  glDeleteVertexArrays(1, &VertexArrayID);
  glDeleteTextures(1, &textureID);
  exit(EXIT_SUCCESS);
}
