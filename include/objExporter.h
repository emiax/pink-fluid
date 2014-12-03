#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

class State;

class ObjExporter {
 public: 
  void exportState(std::string filename, State* state);
 private:
  void printObjToFile(std::string filename, std::vector<glm::vec3> vertices, std::vector<std::vector<int> > faces);
};
