// Include standard headers
#include <stdio.h>
#include <stdlib.h>

//Include for a small timer
#include <ctime>
#include <cmath>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

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
#include <rayCaster.h>
#include <bubbleConfig.h>

// #include <bubbleMaxExporter.h>

int main(int argc, char* argv[]) {

  bool realtimeRendering = false;
  std::string outputDirectory = "";
  bool useBubbleConfig = false;
  std::string bubbleConfigFile = "";
  bool useInitialState = false;
  std::string initialStateFile = "";

  for (int i = 0; i < argc; i++) {
    std::string v = argv[i];
    if (v == "-r") {
      realtimeRendering = true;
    }
    if (v == "-i") {
      if (++i < argc) {
        initialStateFile = std::string(argv[i]);
        useInitialState = true;
        std::cout << "Using initial state in: " << initialStateFile << std::endl;
      } else {
        std::cout << "No state file specified after -i" << std::endl;
      }
    }
    if (v == "-o") {
      if (++i < argc) {
        outputDirectory = std::string(argv[i]) + "/";
        mkdir(argv[1], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        std::cout << "Exporting OBJ files to directory: " << outputDirectory << std::endl;
      } else {
        std::cout << "No directory provided after -o." << std::endl;
      }
    }
    if (v == "-b") {
      // load bubble config from file.
      if (++i < argc) {
        bubbleConfigFile = std::string(argv[i]);
        useBubbleConfig = true;
        std::cout << "Using bubble config in: " << bubbleConfigFile << std::endl;
      } else {
        std::cout << "No config file specified after -b" << std::endl;
      }
    }
  }

  if (outputDirectory == "") {
    std::cout << "exporting OBJ files to current directory" << std::endl;
  }

  srand(time(NULL));

  //Set up the initial state.
  unsigned int w = 32, h = 32, d = 32;
  State initialState(w, h, d);



  // init level set
  LevelSet *ls = factory::levelSet::stairs(w, h, d);
  initialState.setLevelSet(ls);

  delete ls;
  
  VelocityGrid *velocities = new VelocityGrid(w, h, d);
  initialState.setVelocityGrid(velocities);

  if (useInitialState) {
    std::ifstream inputFileStream(initialStateFile, std::ios::binary);
    if(inputFileStream.is_open()){
      initialState.read(inputFileStream);
      inputFileStream.close();
    }
  }
    
  // init simulator
  Simulator sim(initialState, 0.1f);

  BubbleConfig *bubbleConfig = nullptr;

  if (useBubbleConfig) {
    std::ifstream bubbleConfigStream(bubbleConfigFile, std::ios::binary);
    if (bubbleConfigStream.is_open()) {
      bubbleConfig = new BubbleConfig();
      bubbleConfig->read(bubbleConfigStream);
      bubbleConfigStream.close();
    }
  }

  // BubbleMaxExporter bubbleExporter;
  int nbFrames = 0;
  float deltaT = 0.1; //First time step

  ObjExporter objExporter;
  RayCaster *rayCaster = nullptr;

  if (realtimeRendering) {
    rayCaster = new RayCaster();
  }

  int i = 0;
  while (true) {

    // common for both render passes.
    sim.step(deltaT);
    State *currentState = sim.getCurrentState();

    if (bubbleConfig != nullptr) {
      // Manually added bubbles.
      std::vector<Bubble> bs = bubbleConfig->getBubblesInFrame(i);
      sim.addBubbles(bs);
    }

    if (rayCaster != nullptr) {
      glm::mat4 matrix = glm::mat4(1.0f);
      matrix = glm::translate(matrix, glm::vec3(0.0f, 0.0f, 2.0f));
      matrix = glm::rotate(matrix, -3.1415926535f / 4.0f, glm::vec3(1.0f, 0.0f, 0.0f));
      matrix = glm::rotate(matrix, 0.1f * i, glm::vec3(0.0f, 1.0f, 0.0f));
      rayCaster->render(currentState, matrix);
    }

    std::string strDir = std::string(outputDirectory);
    std::string strFrameNumber = std::to_string(i);
    std::string file = strDir + "exported_" + strFrameNumber + ".obj";
    objExporter.exportState(file, currentState);
    std::ofstream fileStream(strDir + "exportedState_" + strFrameNumber + ".pf", std::ios::binary);
    currentState->write(fileStream);
    fileStream.close();

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

  if (bubbleConfig != nullptr) {
    delete bubbleConfig;
  }
  return 0;
}
