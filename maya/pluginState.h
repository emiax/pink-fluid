#pragma once

#include <maya/MFnPlugin.h>

class PluginStateManager;

class PluginState {
public:
  void setExecutionPath(std::string path) {
    executionPath = path;
  }

  std::string getExecutionPath() {
    return executionPath;
  }

  std::string getPluginPath() {
    unsigned found = executionPath.find_last_of("/");
    return executionPath.substr(0, found) + "/maya";
  }

private:
  PluginState() {}
  std::string executionPath;

friend class PluginStateManager;
};

class PluginStateManager {
public:
  PluginStateManager() = delete;
  static PluginState* instance() {
    if (instanceObj == nullptr) {
      instanceObj = new PluginState();
    }
    return instanceObj;
  }

private:
  static PluginState *instanceObj;
};

PluginState* PluginStateManager::instanceObj = nullptr;
