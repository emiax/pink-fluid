#include <maya/MGlobal.h>
#include <maya/MSimple.h>
#include <fstream>
#include <stdio.h>
#include <unistd.h>

class LoadStateCmd : public MPxCommand {
public:
  virtual MStatus doIt(const MArgList& args) {
    MStatus status;
    MString statePath = args.asString(0, &status);

    if (args.length() != 1 || status.error()) {
      MGlobal::displayInfo("usage: pfLoadState(string)");
      return status;
    }

    std::ifstream infile(statePath.asChar());
    if (!infile.good()) {
      MGlobal::displayInfo(MString("Could not load state file: ") + statePath);
    } else {
      MGlobal::displayInfo("State Loaded");
    }

    return status;
  }

  static void* creator() {
    return new LoadStateCmd;
  }
};