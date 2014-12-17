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
#include <fileSequence.h>

// #include <bubbleMaxExporter.h>

int main(int argc, char* argv[]) {

  bool realtimeRendering = false;
  std::string outputDirectory = "";
  bool useBubbleConfig = false;
  std::string bubbleConfigFile = "";
  bool useInitialState = false;
  std::string initialStateFile = "";
  int saveEachNthFrame = 1;
  bool useBubbleSpawning = true;
  bool usePls = true;
  bool shortcut = false;

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
        mkdir(argv[i], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
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

    if (v == "-e") {
      // save each #:th frame
      if (++i < argc) {
        saveEachNthFrame = std::stoi(argv[i]);
        std::cout << "Saving each: " << saveEachNthFrame << " frames" << std::endl;
      } else {
        std::cout << "No config file specified after -b" << std::endl;
      }
    }

    if (v == "-no-pls") {
      usePls = false;
    }

    if (v == "-no-spawning") {
      useBubbleSpawning = false;
    }

    if (v == "-s") {
      shortcut = true;
    }

    if (v == "-h") {
      printf("-r            - show real time ray casted rendering\n");
      printf("-o <dir>      - specify output folder for states\n");
      printf("-b <file>     - specify bubble config file\n");
      printf("-e <#>        - only save each #:th frame\n");
      printf("-i <file>     - load initial state\n");
      printf("-s            - 'shortcut' simulation (read states from files, only simulate bubbles)\n");
      printf("-h            - this help message\n");
      printf("-no-pls       - deactivate particle level set (also deactivates bubble spawning)\n");
      printf("-no-spawning  - deactivate spawning bubbles\n");
      return 0;
    }
  }

  if (outputDirectory == "") {
    std::cout << "exporting OBJ files to current directory" << std::endl;
  }


  FileSequence *fileSequence = nullptr;

  if (shortcut) {
      if (!useInitialState) {
        std::cout << "cannot shortcut simulation without using -i." << std::endl;
        return 0;
      }
      fileSequence = new FileSequence(initialStateFile);
  }

  


  
  srand(time(NULL));

  //Set up the initial state.
  unsigned int w = 16, h = 16, d = 16;
  State initialState(w, h, d);



  // init level set
  LevelSet *ls = factory::levelSet::ball(w, h, d);
  LevelSet *ls2 = factory::levelSet::stairs(w, h, d);
  ls->merge(ls2);

  delete ls2;
  
  initialState.setLevelSet(ls);


  
  VelocityGrid *velocities = new VelocityGrid(w, h, d);
  initialState.setVelocityGrid(velocities);

  if (useInitialState) {
    std::ifstream inputFileStream(initialStateFile, std::ios::binary);
    if(inputFileStream.is_open()){
      initialState.read(inputFileStream);
      inputFileStream.close();
    }
  }


  delete ls;
    
  // init simulator
  Simulator sim(initialState, 0.1f, usePls, useBubbleSpawning);

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
  int savedFrame = 0;


      
  while (true) {
    State *currentState = sim.getCurrentState();
    State cachedState(*currentState);
    
    if (shortcut) {
      std::cout << "shortcut " << i << std::endl;
      std::ifstream inputFileStream(fileSequence->getFileNameRelative(i), std::ios::binary);
      if(inputFileStream.is_open()){
        cachedState.read(inputFileStream);
        std::vector<Bubble> bubbles = currentState->getBubbles();
        cachedState.setBubbles(bubbles);
        sim.setCurrentState(cachedState);
        inputFileStream.close();
      } else {
        std::cout << "no more cached files. disabling shortcutting." << std::endl;
          shortcut = false;
      }
    }
    
    sim.step(deltaT, shortcut);
    
    // refresh current state after taking a simulation step.
    currentState = sim.getCurrentState();

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

    if (i % saveEachNthFrame == 0) {
      std::string strDir = std::string(outputDirectory);
      std::string strFrameNumber = std::to_string(savedFrame);
      std::string file = strDir + "exportedState_" + strFrameNumber + ".pf";

      std::ofstream fileStream(file, std::ios::binary);
      currentState->write(fileStream);
      fileStream.close();

      ++savedFrame;
      std::cout << "Exported frame " << i << " as file: " << file << std::endl;
    }

    std::cout << "Simulated frame " << i << std::endl;
    ++i;
  }

  std::cout << "Cleaning up!" << std::endl;

  if (bubbleConfig != nullptr) {
    delete bubbleConfig;
  }
  return 0;
}
