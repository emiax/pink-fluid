#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <face.h>

class State;
class Mesh;

class ObjExporter {
 public: 
  void exportState(std::string filename, State* state);
 private:
  void printObjToFile(std::string filename, std::vector<glm::vec3> vertices, std::vector<Face > faces);
  void printMeshToFile(std::string filename, Mesh &mesh);
};
