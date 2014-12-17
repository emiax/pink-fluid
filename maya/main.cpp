#include "pluginState.h"
#include "mayaVelocityGrid.h"

// commands
#include "setGridResolutionCmd.h"
#include "loadStateCmd.h"
#include "launchGuiCmd.h"
#include "setBubblesCmd.h"
#include "removeBubblesCmd.h"
#include "simulateCmd.h"

MStatus initializePlugin(MObject obj) {
  MFnPlugin pluginFn(obj, "PinkFluid", "0.1");
  MGlobal::displayInfo(MString("Loaded PinkFluid from: ") + pluginFn.loadPath());
  PluginStateManager::instance()->setExecutionPath(pluginFn.loadPath().asChar());

  MStatus status;
  status = pluginFn.registerCommand("pfSetGridResolution", SetGridResolutionCmd::creator);
  status = pluginFn.registerCommand("pfLoadState", LoadStateCmd::creator);
  status = pluginFn.registerCommand("pfGUI", LaunchGuiCmd::creator);
  status = pluginFn.registerCommand("pfSetBubbles", SetBubblesCmd::creator);
  status = pluginFn.registerCommand("pfRemoveBubbles", RemoveBubblesCmd::creator);
  status = pluginFn.registerCommand("pfSimulate", SimulateCmd::creator);

  MGlobal::executeCommand("pfGUI");

  MString path(PluginStateManager::instance()->getPluginPath().c_str());
  MGlobal::sourceFile(path + "/objSequenceImporter.mel");
  MGlobal::sourceFile(path + "/connectStates.mel");

  return status;
}

MStatus uninitializePlugin(MObject obj) {
  MFnPlugin pluginFn(obj);
  MStatus status;
  status = pluginFn.deregisterCommand("pfSetGridResolution");
  status = pluginFn.deregisterCommand("pfLoadState");
  status = pluginFn.deregisterCommand("pfGUI");
  status = pluginFn.deregisterCommand("pfSetBubbles");
  status = pluginFn.deregisterCommand("pfRemoveBubbles");
  status = pluginFn.deregisterCommand("pfSimulate");

  MGlobal::executeCommand("deleteUI $win");

  return status;
}
