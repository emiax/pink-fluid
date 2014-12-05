#include <objExporter.h>
#include <ordinalGrid.h>
#include <state.h>
#include <fstream>
#include <sdfTessellation.h>
#include <bubbleTessellation.h>
#include <mesh.h>


void ObjExporter::exportState(std::string filename, State *state) {
  std::vector<glm::vec3> vertexList;
  std::vector<Face > faceIndices;

  const OrdinalGrid<float> *sdf = state->getSignedDistanceGrid();

  Mesh m;

  SdfTessellation sdfTess(sdf);
  m.addData(sdfTess.getVertices(), sdfTess.getFaces());

  std::vector<Bubble> bubbles = state->getBubbles();
  for (int i = 0; i < bubbles.size(); ++i) {
    BubbleTessellation bt(&bubbles[i]);
    m.addData(bt.getVertices(), bt.getFaces());
  }
  
  printMeshToFile(filename, m);
}


void ObjExporter::printObjToFile(std::string filename, std::vector<glm::vec3> vertices, std::vector<Face > faces) {
  std::ofstream outputFile(filename);
  for(auto vertex : vertices){
    outputFile << "v " << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
  }
  for(auto face : faces) {
    outputFile << "f " << face[0] + 1 << " " << face[1] + 1 << " " << face[2] + 1 << std::endl;
  }
  outputFile.close();
}


void ObjExporter::printMeshToFile(std::string filename, Mesh& m) {
  printObjToFile(filename, m.getVertices(), m.getFaces());
}


