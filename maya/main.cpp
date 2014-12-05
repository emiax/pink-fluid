#include "pluginState.h"
#include "mayaVelocityGrid.h"

// commands
#include "setGridResolutionCmd.h"
#include "loadStateCmd.h"
#include "launchGuiCmd.h"
#include "setBubblesCmd.h"

MStatus initializePlugin(MObject obj) {
  MFnPlugin pluginFn(obj, "PinkFluid", "0.1");
  MGlobal::displayInfo(MString("Loaded PinkFluid from: ") + pluginFn.loadPath());
  PluginStateManager::instance()->setExecutionPath(pluginFn.loadPath().asChar());

  MStatus status;
  status = pluginFn.registerCommand("pfSetGridResolution", SetGridResolutionCmd::creator);
  status = pluginFn.registerCommand("pfLoadState", LoadStateCmd::creator);
  status = pluginFn.registerCommand("pfGUI", LaunchGuiCmd::creator);
  status = pluginFn.registerCommand("pfSetBubbles", SetBubblesCmd::creator);

  MGlobal::executeCommand("pfGUI");

  MString path(PluginStateManager::instance()->getPluginPath().c_str());
  MGlobal::sourceFile(path + "/objSequenceImporter.mel");

  return status;
}

MStatus uninitializePlugin(MObject obj) {
  MFnPlugin pluginFn(obj);
  MStatus status;
  status = pluginFn.deregisterCommand("pfSetGridResolution");
  status = pluginFn.deregisterCommand("pfLoadState");
  status = pluginFn.deregisterCommand("pfGUI");
  status = pluginFn.deregisterCommand("pfSetBubbles");

  MGlobal::executeCommand("deleteUI $win");

  return status;
}
