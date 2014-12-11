#pragma once

// Maya
#include <maya/MGlobal.h>
#include <maya/MSimple.h>
#include <maya/MSelectionList.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnTransform.h>
#include <maya/MFnParticleSystem.h>
#include <maya/MIOstream.h>
#include <maya/MVectorArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MMatrix.h>
#include <maya/MTransformationMatrix.h>

// Pink-Fluid + deps
#include <bubbleConfig.h>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

#include "pluginState.h"

class RemoveBubblesCmd : public MPxCommand {
public:
  static void* creator() {
    return new RemoveBubblesCmd;
  }

  virtual MStatus doIt(const MArgList& args) {
    MStatus status;
    int frameNumber = args.asInt(0, &status);
    if (status.error()) {
      MGlobal::displayInfo(MString("invalid arguments for RemoveBubbles: ") + status.errorString());
      return status;
    }

    // Update conf file
    const std::string BUBBLE_CONF_FILE_NAME = PluginStateManager::instance()->getExecutionPath() + "/bubbleConf.pf";
    cout << "file path = " << BUBBLE_CONF_FILE_NAME << endl;
    std::ifstream bubbleConfigInFile(BUBBLE_CONF_FILE_NAME);
    BubbleConfig bubbleConfig;
    if (bubbleConfigInFile.fail()) {
      cout << "There's no bubble conf file with path: " << BUBBLE_CONF_FILE_NAME << endl;
      return status;
    }
    bubbleConfig.read(bubbleConfigInFile);
    bubbleConfig.removeBubbles(frameNumber);
    std::ofstream bubbleConfigOutFile(BUBBLE_CONF_FILE_NAME);
    bubbleConfig.write(bubbleConfigOutFile);

    return status;
  }
};