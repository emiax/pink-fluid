// Include standard headers
#include <stdio.h>
#include <stdlib.h>

//Include for a small timer
#include <ctime>
#include <cmath>

#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

// Include GLM
#include <glm/glm.hpp>

// io
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

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
// #include <bubbleMaxExporter.h>

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

int main(int argc, char* argv[]) {
  srand(time(NULL));

    //Set up the initial state.
  unsigned int w = 32, h = 32, d = 32;
  State *prevState = new State(w, h, d);
  State *newState = new State(w, h, d);

  VelocityGrid *velocities = new VelocityGrid(w, h, d);
  prevState->setVelocityGrid(velocities);

    // init level set
  LevelSet *ls = factory::levelSet::fourthContainerBoxInFluid(w, h, d);
  prevState->setLevelSet(ls);
  newState->setLevelSet(ls);

  delete ls;

    // init simulator
  Simulator sim(prevState, newState, 0.1f);
    // BubbleMaxExporter bubbleExporter;
  int nbFrames = 0;
    float deltaT = 0.1; //First time step

    std::string dir = "";
    if (argc > 1) {
      dir = std::string(argv[1]) + "/";
      mkdir(argv[1], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      std::cout << "exporting OBJ files to directory: " << dir << std::endl;
    } else {
      std::cout << "exporting OBJ files to current directory" << std::endl;
    }

    int i = 0;
    while (true) {
        // common for both render passes.
      sim.step(deltaT);
      std::swap(prevState, newState);

      std::vector<glm::vec3> vertexList;
      std::vector<std::vector<int> > faceIndices;
        // marching cubes
      for (unsigned int k = 0; k < d; ++k) {
        for (unsigned int j = 0; j < h; ++j) {
          for (unsigned int i = 0; i < w; ++i) {
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

            marchingCubes::TRIANGLE *triangles = new marchingCubes::TRIANGLE[5];
            int numTriangles = marchingCubes::PolygoniseCube(gridcell, 0.0, triangles);
            for(int i = 0; i < numTriangles; i++){
              int startIndex = vertexList.size()+1;
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
        }
      }
    }

    std::string file = std::string(dir) + "exported_" + std::to_string(i) + ".obj";
    printObjToFile(file, vertexList, faceIndices);
        // std::ofstream fileStream("exportedState_" + std::to_string(i) + ".pf", std::ios::binary);
        // newState->write(fileStream);
        // fileStream.close();

        // bubbleExporter.update(i, sim.getBubbleTracker());
        // bubbleExporter.exportSnapshot(i, "bubbles_" + std::to_string(i) + ".mx");

        // if (i > 600) {
        //   bubbleExporter.exportBubbles("bubbles.mx");
        //   break;
        // }
    std::cout << "Exported OBJ file: " << file << std::endl;
    ++i;
  }

  std::cout << "Cleaning up!" << std::endl;
  return 0;
}
