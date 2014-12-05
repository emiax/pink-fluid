#include <mesh.h>

void Mesh::addData(std::vector<glm::vec3> vertices, std::vector<Face> faces) {
  int offset = this->vertices.size();

  // copy all vertex coordinates.
  for (int i = 0; i < vertices.size(); ++i) {
    this->vertices.push_back(vertices[i]);
  }
  
  for (int i = 0; i < faces.size(); ++i) {
    this->faces.push_back(Face(faces[i].x + offset, faces[i].y + offset, faces[i].z + offset));
  }
}

std::vector<glm::vec3> Mesh::getVertices() {
  return vertices;
}

std::vector<Face> Mesh::getFaces() {
  return faces;
}
