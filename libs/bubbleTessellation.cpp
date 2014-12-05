#include <bubbleTessellation.h>
#include <bubbleTracker.h>
#include <face.h>

BubbleTessellation::BubbleTessellation(Bubble *bubble) {
  this->bubble = bubble;
}

void BubbleTessellation::tessellate() {
  tessellated = true;
  // Todo: implement this function.
}

std::vector<glm::vec3> BubbleTessellation::getVertices() {
  if (!tessellated) {
    tessellate();
  }
  return vertices;
}

std::vector<Face> BubbleTessellation::getFaces() {
  if (!tessellated) {
    tessellate();
  }
  return faces;
}
