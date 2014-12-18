#include <factories/levelSetFactories.h>
#include <simulator.h>
class SimulateCmd : public MPxCommand {
public:
  
  virtual MStatus doIt(const MArgList& args) {
    MStatus status;
    int startSimulateFrame = args.asInt(0, &status);
    int endSimulateFrame = args.asInt(1, &status);

    unsigned int w = 32, h = 32, d = 32;
    State initialState(w, h, d);

    LevelSet *ls = factory::levelSet::ball(w, h, d);
    
    initialState.setLevelSet(ls);

    delete ls;

    Simulator simulator(initialState, 0.1f, true, true);

    StateImporter importer;
    
    std::vector<std::string> meshNames;
    std::vector<int> frameNumbers;
    std::string createProgressCommand = "progressWindow  -status \"Simulating Fluid..\" -maxValue " + std::to_string(endSimulateFrame) + " -title \"Simulation\" -isInterruptable true;";
    MGlobal::executeCommand(createProgressCommand.c_str());
    std::cout << startSimulateFrame << std::endl;
    std::cout << endSimulateFrame << std::endl;
    for(int i = 0; i < endSimulateFrame; i++){
      int result;
      MGlobal::executeCommand("progressWindow -query -isCancelled", result);
      std::cout << std::endl;
      if(result){
        break;
      }
      simulator.step(0.1);
      State *state = simulator.getCurrentState();
      std::cout << state << std::endl;
      auto stateNames = importer.importState(state, i);
      MGlobal::displayInfo("State Loaded");
      meshNames.push_back(stateNames.meshName);
      frameNumbers.push_back(state->getFrameNumber());
      MGlobal::executeCommand("progressWindow -e -step 1;");
    }
    MGlobal::executeCommand("progressWindow -endProgress;");
    importer.connectStates(meshNames, frameNumbers);
    return status;
  }
  static void* creator() {
    return new SimulateCmd;
  }
};
