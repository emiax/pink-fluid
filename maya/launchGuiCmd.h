#include <maya/MGlobal.h>
#include <maya/MSimple.h>
#include <fstream>
#include "pluginState.h"

class LaunchGuiCmd : public MPxCommand {
public:
  virtual MStatus doIt(const MArgList&) {
    MStatus status;
    MString path(PluginStateManager::instance()->getPluginPath().c_str());
    MGlobal::sourceFile(path + "/gui.mel");

    return status;
  }

  static void* creator() {
    return new LaunchGuiCmd;
  }
};