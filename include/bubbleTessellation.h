#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <face.h>

class Bubble;

class BubbleTessellation {
public:
  BubbleTessellation(Bubble *b);
  std::vector<glm::vec3> getVertices();
  std::vector<Face> getFaces();
private:
  Bubble *bubble;
  void tessellate();
  bool tessellated = false;
  std::vector<glm::vec3> vertices;
  std::vector<Face> faces;
};
