#include <objExporter.h>
#include <ordinalGrid.h>
#include <state.h>
#include <marchingcubes/marchingcubes.h>
#include <fstream>

void ObjExporter::exportState(std::string filename, State *state) {
  std::vector<glm::vec3> vertexList;
  std::vector<std::vector<int> > faceIndices;

  const OrdinalGrid<float> *sdf = state->getSignedDistanceGrid();
  int w = sdf->getW();
  int h = sdf->getH();
  int d = sdf->getD();

  // marching cubes
  for (unsigned int k = 0; k < d; ++k) {
    for (unsigned int j = 0; j < h; ++j) {
      for (unsigned int i = 0; i < w; ++i) {
        if(sdf->isValid(i+1,j,k) &&
           sdf->isValid(i,j+1,k) &&
           sdf->isValid(i+1,j+1,k) &&
           sdf->isValid(i,j,k+1) &&
           sdf->isValid(i+1,j,k+1) &&
           sdf->isValid(i,j+1,k+1) &&
           sdf->isValid(i+1,j+1,k+1)) {
          
          marchingCubes::GRIDCELL gridcell;
          gridcell.p[0] = glm::vec3(i,j,k);
          gridcell.p[1] = glm::vec3(i,j+1,k);
          gridcell.p[2] = glm::vec3(i+1,j+1,k);
          gridcell.p[3] = glm::vec3(i+1,j,k);
          gridcell.p[4] = glm::vec3(i,j,k+1);
          gridcell.p[5] = glm::vec3(i,j+1,k+1);
          gridcell.p[6] = glm::vec3(i+1,j+1,k+1);
          gridcell.p[7] = glm::vec3(i+1,j,k+1);

          gridcell.val[0] = sdf->get(i, j, k);
          gridcell.val[1] = sdf->get(i, j+1, k);
          gridcell.val[2] = sdf->get(i+1, j+1, k);
          gridcell.val[3] = sdf->get(i+1, j, k);
          gridcell.val[4] = sdf->get(i, j, k+1);
          gridcell.val[5] = sdf->get(i, j+1, k+1);
          gridcell.val[6] = sdf->get(i+1, j+1, k+1);
          gridcell.val[7] = sdf->get(i+1, j, k+1);

          marchingCubes::TRIANGLE *triangles = new marchingCubes::TRIANGLE[5];
          int numTriangles = marchingCubes::PolygoniseCube(gridcell, 0.0, triangles);
          for(int i = 0; i < numTriangles; i++){
            int startIndex = vertexList.size()+1;
            vertexList.push_back(triangles[i].p[0]);
            vertexList.push_back(triangles[i].p[1]);
            vertexList.push_back(triangles[i].p[2]);
            
            std::vector<int> indices = {
              startIndex,
              startIndex+1,
              startIndex+2
            };
            
            faceIndices.push_back(indices);
          }
          
          delete[] triangles;
        }
      }
    }
  }
  printObjToFile(filename, vertexList, faceIndices);
}


void ObjExporter::printObjToFile(std::string filename, std::vector<glm::vec3> vertices, std::vector<std::vector<int> > faces) {
  std::ofstream outputFile(filename);
  for(auto vertex : vertices){
    outputFile << "v " << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
  }
  for(auto face : faces) {
    outputFile << "f ";
    for (auto faceIndex : face) {
      outputFile << faceIndex << " ";
    }
    outputFile << std::endl;
  }
  outputFile.close();
}

