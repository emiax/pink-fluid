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
#include <objExporter.h>

// #include <bubbleMaxExporter.h>

int main(int argc, char* argv[]) {
  srand(time(NULL));

  //Set up the initial state.
  unsigned int w = 32, h = 32, d = 32;
  State initialState(w, h, d);

  VelocityGrid *velocities = new VelocityGrid(w, h, d);
  initialState.setVelocityGrid(velocities);

  // init level set
  LevelSet *ls = factory::levelSet::fourthContainerBoxInFluid(w, h, d);
  initialState.setLevelSet(ls);

  delete ls;

  // init simulator
  Simulator sim(initialState, 0.1f);

  std::ifstream inputFileStream("states/exportedState_" + std::to_string(70) + ".pf", std::ios::binary);
  if(inputFileStream.is_open()){
    std::cout << "Read state" << std::endl;
    initialState.read(inputFileStream);
    inputFileStream.close();
  }

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

  ObjExporter objExporter;

  int i = 0;
  while (true) {

    // common for both render passes.
    sim.step(deltaT);
    State *currentState = sim.getCurrentState();
    
    std::string file = std::string(dir) + "exported_" + std::to_string(i) + ".obj";
    objExporter.exportState(file, currentState);
    // std::ofstream fileStream("exportedState_" + std::to_string(i) + ".pf", std::ios::binary);
    // currentState->write(fileStream);
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
