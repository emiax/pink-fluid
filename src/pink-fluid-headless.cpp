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

#include <utility>
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
#include <common/FBO.h>
#include <factories/levelSetFactories.h>
#include <ordinalGrid.h>
#include <state.h>
#include <simulator.h>
#include <velocityGrid.h>
#include <signedDistanceFunction.h>
#include <levelSet.h>
#include <stdlib.h>
#include <time.h>
#include <bubbleTracker.h>
#include <bubbleMaxExporter.h>

#include <marchingcubes/marchingcubes.h>

void printObjToFile(std::string filename, std::vector<glm::vec3> vertices, std::vector<std::vector<int> > faces){
    std::ofstream outputFile(filename);
    for(auto vertex : vertices){
        outputFile << "v " << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
    }
    for(auto face : faces) {
        outputFile << "f ";
        for (auto faceIndex : face) {
            outputFile << faceIndex << " ";
        }
        outputFile << std::endl;
    }
    outputFile.close();
}

int main(void) {
    srand(time(NULL));

    //Create init object
    Init init = Init();

    //Initialize glfw
    init.glfw(4, 1);
    //Open a window
    GLFWwindow *window = init.window(400, 400);

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
    unsigned int w = 64, h = 64, d = 64;
    State *prevState = new State(w, h, d);
    State *newState = new State(w, h, d);

    VelocityGrid *velocities = new VelocityGrid(w, h, d);
    prevState->setVelocityGrid(velocities);


    // init level set
    LevelSet *ls = factory::levelSet::ball(w, h, d);
    prevState->setLevelSet(ls);
    newState->setLevelSet(ls);

    delete ls;

    // init simulator
    Simulator sim(prevState, newState, 0.1f);
    BubbleMaxExporter bubbleExporter;
    
    // Dark black background
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    //Load in shaders
    static ShaderProgram colorCubeProg("../vertShader.vert", "../colorCube.frag");
    static ShaderProgram rayCasterProg("../vertShader.vert", "../rayCaster.frag");
    static ShaderProgram bubbleProg("../bubbleVertShader.vert", "../bubbleFragShader.frag");

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

    std::vector<GLfloat> g_bubble_buffer_data;

    //Create vertex buffer
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData, GL_STATIC_DRAW);



    GLuint triangleBuffer;
    glGenBuffers(1, &triangleBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangleBufferData), triangleBufferData, GL_STATIC_DRAW);

    // Create bubble buffer
    GLuint bubbleBuffer;
    glGenBuffers(1, &bubbleBuffer);

    // Create framebuffer
    FBO *framebuffer = new FBO(width, height);

    GLuint volumeTextureId;
    glGenTextures(1, &volumeTextureId);
    glBindTexture(GL_TEXTURE_3D, volumeTextureId);

    //Object which encapsulates a texture + The destruction of a texture.
    Texture3D tex3D(w, h, d);
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    float deltaT = 0.1; //First time step

    glfwSwapInterval(1);
    int i = 0;
    do {


        framebuffer->activate();

        // common for both render passes.
        sim.step(deltaT);

        // deltaT = sim.getDeltaT();

        glm::mat4 matrix = glm::mat4(1.0f);
        matrix = glm::translate(matrix, glm::vec3(0.0f, 0.0f, 2.0f));
        matrix = glm::rotate(matrix, -3.1415926535f / 4.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        matrix = glm::rotate(matrix, 0.1415926535f / 4.0f * (float) glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));

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

        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
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
        glBindTexture(GL_TEXTURE_2D, *(framebuffer->getTexture()));

        GLuint textureLocation = glGetUniformLocation(rayCasterProg, "backfaceTexture");
        glUniform1i(textureLocation, 0);

        
        // ping pong states
        std::swap(prevState, newState);

        std::vector<glm::vec3> vertexList;
        std::vector<std::vector<int> > faceIndices;
        // copy desired quantities to texture
        for (unsigned int k = 0; k < d; ++k) {
            for (unsigned int j = 0; j < h; ++j) {
                for (unsigned int i = 0; i < w; ++i) {

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

                    if(newState->getSignedDistanceGrid()->isValid(i+1,j,k) &&
                        newState->getSignedDistanceGrid()->isValid(i,j+1,k) &&
                        newState->getSignedDistanceGrid()->isValid(i+1,j+1,k) &&
                        newState->getSignedDistanceGrid()->isValid(i,j,k+1) &&
                        newState->getSignedDistanceGrid()->isValid(i+1,j,k+1) &&
                        newState->getSignedDistanceGrid()->isValid(i,j+1,k+1) &&
                        newState->getSignedDistanceGrid()->isValid(i+1,j+1,k+1)){

                        marchingCubes::GRIDCELL gridcell;
                        gridcell.p[0] = glm::vec3(i,j,k);
                        gridcell.p[1] = glm::vec3(i,j+1,k);
                        gridcell.p[2] = glm::vec3(i+1,j+1,k);
                        gridcell.p[3] = glm::vec3(i+1,j,k);
                        gridcell.p[4] = glm::vec3(i,j,k+1);
                        gridcell.p[5] = glm::vec3(i,j+1,k+1);
                        gridcell.p[6] = glm::vec3(i+1,j+1,k+1);
                        gridcell.p[7] = glm::vec3(i+1,j,k+1);

                        gridcell.val[0] = newState->getSignedDistanceGrid()->get(i, j, k);
                        gridcell.val[1] = newState->getSignedDistanceGrid()->get(i, j+1, k);
                        gridcell.val[2] = newState->getSignedDistanceGrid()->get(i+1, j+1, k);
                        gridcell.val[3] = newState->getSignedDistanceGrid()->get(i+1, j, k);
                        gridcell.val[4] = newState->getSignedDistanceGrid()->get(i, j, k+1);
                        gridcell.val[5] = newState->getSignedDistanceGrid()->get(i, j+1, k+1);
                        gridcell.val[6] = newState->getSignedDistanceGrid()->get(i+1, j+1, k+1);
                        gridcell.val[7] = newState->getSignedDistanceGrid()->get(i+1, j, k+1);

                        //std::cout << gridcell.val[0] << std::endl;

                        marchingCubes::TRIANGLE *triangles = new marchingCubes::TRIANGLE[5];
                        int numTriangles = marchingCubes::PolygoniseCube(gridcell, 0.0, triangles);
                        for(int i = 0; i < numTriangles; i++){
                            int startIndex = vertexList.size()+1;
                            for(int j = 0; j < 3; j++){
                                //std::cout << triangles[i].p[j].x << " " << triangles[i].p[j].y << " " << triangles[i].p[j].z << std::endl;
                            }
                            vertexList.push_back(triangles[i].p[0]);
                            vertexList.push_back(triangles[i].p[1]);
                            vertexList.push_back(triangles[i].p[2]);

                            std::vector<int> indices = {
                                    startIndex,
                                    startIndex+1,
                                    startIndex+2
                            };

                            faceIndices.push_back(indices);
                        }

                        delete[] triangles;
                    }
                    //signed dist
                    float dist = newState->getSignedDistanceGrid()->get(i, j, k);
                    float solid = newState->getCellTypeGrid()->get(i, j, k) == CellType::SOLID ? 1.0f : 0.0f;
                    dist = (glm::clamp(dist + solid, -1.0f, 1.0f) + 1) / 2;

                    tex3D.set(i, j, k, 0, solid);
                    tex3D.set(i, j, k, 1, 0.0f); // not used
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

        printObjToFile("exported_" + std::to_string(i) + ".obj", vertexList, faceIndices);
        std::ofstream fileStream("exportedState_" + std::to_string(i) + ".pf", std::ios::binary);
        newState->write(fileStream);
        fileStream.close();


        // activate and upload texture to gpu
        tex3D(GL_TEXTURE1);
        GLuint volumeTextureLocation = glGetUniformLocation(rayCasterProg, "volumeTexture");
        glUniform1i(volumeTextureLocation, 1);

        glEnableVertexAttribArray(0);
        glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, 0);
        glDisableVertexAttribArray(0);

        FBO::deactivate();

        

        ////////////////// Start drawing bubbles //////////////////////
        
        // Draw bubbles
        const std::vector<Bubble*> *bubbles = sim.getBubbleTracker()->getBubbles();
        g_bubble_buffer_data.clear();
        std::cout << "frame=" << i << ", nBubbles=" << bubbles->size() << std::endl;
        for (int i = 0; i < bubbles->size(); i++) {
          Bubble b = *bubbles->at(i);

          //          std::cout << "bubble pos " << b.position.x << ", " << b.position.y << std::endl << b.radius << std::endl;
            
          g_bubble_buffer_data.push_back(b.position.x / (float)w * 2.0 - 1.0);
          g_bubble_buffer_data.push_back(b.position.y / (float)h * 2.0 - 1.0);
          g_bubble_buffer_data.push_back(b.position.z / (float)d * 2.0 - 1.0);
          g_bubble_buffer_data.push_back(b.radius);
        }

           
        glBindBuffer(GL_ARRAY_BUFFER, bubbleBuffer);    
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * g_bubble_buffer_data.size(), &g_bubble_buffer_data[0], GL_DYNAMIC_DRAW);
        
                
        
        bubbleProg();
        glEnable(GL_PROGRAM_POINT_SIZE);

        {
            GLuint tLocation = glGetUniformLocation(colorCubeProg, "time");
            glUniform1f(tLocation, glfwGetTime());

            GLuint mvLocation = glGetUniformLocation(colorCubeProg, "mvMatrix");
            glUniformMatrix4fv(mvLocation, 1, false, glm::value_ptr(matrix));
        }

        //        glEnable (GL_BLEND);
        //        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                
        glPointSize(4.0);
        
        if (g_bubble_buffer_data.size() > 0) {
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
          
          glDrawArrays(GL_POINTS, 0, 4 * g_bubble_buffer_data.size()); // 3 indices starting at 0 -> 1 triangle
          glDisableVertexAttribArray(0);
        }
        ////////////////// End drawing bubbles //////////////////////
        




        glfwPollEvents();
        glfwSwapBuffers(window);
        double currentTime = glfwGetTime();
        nbFrames++;
        if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1 sec ago
            // printf and reset timer
            std::string title = std::to_string(1000.0 / double(nbFrames)) + "ms/frame        " + std::to_string(deltaT) + "  dt";
            glfwSetWindowTitle(window, title.c_str());
            nbFrames = 0;
            lastTime += 1.0;
        }
        i++;
        
        bubbleExporter.update(i, sim.getBubbleTracker());
        bubbleExporter.exportSnapshot(i, "bubbles_" + std::to_string(i) + ".mx");

        if (i > 600) {
          bubbleExporter.exportBubbles("bubbles.mx");
          break;
        }
    } // Check if the ESC key was pressed or the window was closed
    while (!glfwWindowShouldClose(window));

    std::cout << "Cleaning up!" << std::endl;
    // Close OpenGL window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    glDeleteBuffers(1, &vertexbuffer);

    glDeleteVertexArrays(1, &VertexArrayID);
    exit(EXIT_SUCCESS);
}
