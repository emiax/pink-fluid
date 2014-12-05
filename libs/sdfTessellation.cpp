#include <sdfTessellation.h>
#include <marchingcubes/marchingcubes.h>

SdfTessellation::SdfTessellation(const OrdinalGrid<float> *sdf) {
  this->sdf = sdf;
}


std::vector<glm::vec3> SdfTessellation::getVertices() {
  if (!tessellated) {
    tessellate();
  }
  return vertices;
}

std::vector<Face > SdfTessellation::getFaces() {
  if (!tessellated) {
    tessellate();
  }
  return faces;
}

void SdfTessellation::tessellate() {
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

          marchingCubes::TRIANGLE *cellTriangles = new marchingCubes::TRIANGLE[5];
          int numCellTriangles = marchingCubes::PolygoniseCube(gridcell, 0.0, cellTriangles);
          for(int i = 0; i < numCellTriangles; i++){
            int startIndex = vertices.size();
            vertices.push_back(cellTriangles[i].p[0]);
            vertices.push_back(cellTriangles[i].p[1]);
            vertices.push_back(cellTriangles[i].p[2]);
            
            Face face(startIndex, startIndex+1, startIndex+2);
            faces.push_back(face);
          }
          
          delete[] cellTriangles;
        }
      }
    }
  }
  tessellated = true;
}


