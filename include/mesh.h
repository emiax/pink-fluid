#include <glm/glm.hpp>
#include <vector>
#include <face.h>

class Mesh {
 public:
  void addData(std::vector<glm::vec3> vertices, std::vector<Face> faces);
  std::vector<glm::vec3> getVertices();
  std::vector<Face> getFaces();
 private:
  std::vector<glm::vec3> vertices;
  std::vector<Face> faces;
};
