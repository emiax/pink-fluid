#include <maya/MGlobal.h>
#include <maya/MSimple.h>
#include <fstream>
#include <state.h>
#include <stdio.h>
#include <unistd.h>

class LoadStateCmd : public MPxCommand {
public:
  virtual MStatus doIt(const MArgList& args) {
    MStatus status;
    MString statePath = args.asString(0, &status);
    if(state){
      delete state;
      state = nullptr;
    }
    state = new State(1,1,1);
    if (args.length() != 1 || status.error()) {
      return status;
    }

    std::ifstream inputFileStream(statePath.asChar(), std::ios::binary);
    if(inputFileStream.good()){
      state->read(inputFileStream);
      inputFileStream.close();
      MGlobal::displayInfo("State Loaded");
    }
    else{
      MGlobal::displayInfo(MString("Could not load state file: ") + statePath);
    }

    return status;
  }

  static void* creator() {
    return new LoadStateCmd;
  }

  State *state = nullptr;
};
