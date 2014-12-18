#include <objExporter.h>
#include "stateImporter.h"
#include <fstream>
#include <state.h>
#include <stdio.h>
#include <unistd.h>
#include <face.h>
#include <sstream>

class LoadStateCmd : public MPxCommand {
public:
  
  virtual MStatus doIt(const MArgList& args) {
    MStatus status;
    unsigned int index = 0;
    const MStringArray pathArray = args.asStringArray(index, &status);

    State *state = new State(1,1,1);
    if (args.length() != 1 || status.error()) {
      return status;
    }
    StateImporter importer;
    std::vector<std::string> meshNames;
    std::vector<int> frameNumbers;
    MString numMeshesString = std::to_string(pathArray.length()).c_str();
    std::string createProgressCommand = "progressWindow  -status \"Importing State Sequence...\" -maxValue " + std::to_string(pathArray.length()) + " -title \"Importing\" -isInterruptable true;";
    MGlobal::executeCommand(createProgressCommand.c_str());
    for(int i = 0; i < pathArray.length(); i++){
      std::ifstream inputFileStream(pathArray[i].asChar(), std::ios::binary);
      int result;
      MGlobal::executeCommand("progressWindow -query -isCancelled", result);
      if(result){
        break;
      }
      if(inputFileStream.good()){
        state->read(inputFileStream);
        inputFileStream.close();
        auto stateNames = importer.importState(state, i);
        MGlobal::displayInfo("State Loaded");
        meshNames.push_back(stateNames.meshName);
        frameNumbers.push_back(state->getFrameNumber());
      }
      else{
        MGlobal::displayInfo(MString("Could not load state file: ") + pathArray[i]);
      }
      MGlobal::executeCommand("progressWindow -e -step 1;");
    }
    MGlobal::executeCommand("progressWindow -endProgress;");
    importer.connectStates(meshNames, frameNumbers);
    return status;
  }

  static void* creator() {
    return new LoadStateCmd;
  }
};
