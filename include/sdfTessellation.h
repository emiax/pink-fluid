#pragma once
#include <glm/glm.hpp>
#include <face.h>
#include <ordinalGrid.h>
#include <vector>

class SdfTessellation {
 public:
  SdfTessellation(const OrdinalGrid<float> *sdf);
  std::vector<glm::vec3> getVertices();
  std::vector<Face> getFaces();
 private: 
  const OrdinalGrid<float> *sdf;
  void tessellate();
  bool  tessellated = false;
  std::vector<glm::vec3> vertices;
  std::vector<Face> faces;
};
