#include <levelSet.h>
#include <glm/ext.hpp>
#include <gridHeap.h>
#include <iostream>

LevelSet::LevelSet(unsigned int w, unsigned int h, unsigned int d, SignedDistanceFunction sdf, Grid<CellType> const* const ctg){
  this->w = w;
  this->h = h;
  this->d = d;

  doneGrid = new Grid<bool>(w, h, d);
  distanceGrid = new OrdinalGrid<float>(w, h, d);
  cellTypeGrid = new Grid<CellType>(w, h, d);
  this->initSDF = new SignedDistanceFunction(sdf.getFunction());

  gridHeap = new GridHeap(w, h, d, distanceGrid);
  closestPointGrid = new Grid<glm::vec3>(w, h, d);

  setCellTypeGrid(ctg);
  initializeDistanceGrid(sdf);
  updateCellTypes();
}


LevelSet::~LevelSet() {
  delete doneGrid;
  delete distanceGrid;
  delete cellTypeGrid;
  delete this->initSDF;
  delete gridHeap;
  delete closestPointGrid;
}


void LevelSet::reinitialize() {
  gridHeap->clear();
  updateInterfaceNeighbors();
  fastMarch();
  updateCellTypes();
  clampInfiniteCells();
}

void LevelSet::updateInterfaceNeighbors(){

  for(unsigned k = 0; k < d; ++k) {
    for(unsigned j = 0; j < h; ++j) {
      for(unsigned i = 0; i < w; ++i) {
        //Currently only creates the levelset-based on fluid
        CellType currentCellType = cellTypeGrid->get(i,j,k);
        updateInterfaceNeighborCell(i, j, k);
      }
    }
  }

  for(unsigned k = 0; k < d; ++k) {
    for(unsigned j = 0; j < h; ++j) {
      for(unsigned i = 0; i < w; ++i) {
        if (glm::abs(distanceGrid->get(i, j, k)) < INF) {
          updateNeighborsFrom(GridCoordinate(i, j, k));
        }
      }
    }
  }
}


void LevelSet::clampInfiniteCells() {
  for(auto k = 0u; k < d; k++) {
    for(auto j = 0u; j < h; j++) {
      for(auto i = 0u; i < w; i++) {
        if (distanceGrid->get(i, j, k) > 5.0) {
          distanceGrid->set(i, j, k, 5.0);
        }
        if (distanceGrid->get(i, j, k) < -5.0) {
          distanceGrid->set(i, j, k, -5.0);
        }
      }
    }
  }
}


void LevelSet::updateInterfaceNeighborCell(unsigned int i, unsigned int j, unsigned int k) {
  float current = distanceGrid->get(i, j, k);
  float east = distanceGrid->clampGet(i+1, j, k);
  float west = distanceGrid->clampGet(i-1, j, k);
  float north = distanceGrid->clampGet(i, j-1, k);
  float south = distanceGrid->clampGet(i, j+1, k);
  float down = distanceGrid->clampGet(i, j, k-1);
  float up = distanceGrid->clampGet(i, j, k+1);

  int currentCellSign = sgn(current);
  
  float dist = INF;
  glm::vec3 closestPoint(i, j, k);

  
  if (sgn(east) != currentCellSign) {
    float distCandidate = - current / (east - current);
    if (distCandidate < dist) {
      dist = distCandidate;
      closestPoint = glm::vec3(i + dist, j, k);
    }
  }
  if (sgn(west) != currentCellSign) {
    float distCandidate = - current / (west - current);
    if (distCandidate < dist) {
      dist = distCandidate;
      closestPoint = glm::vec3(i - dist, j, k);
    }
  }
  if (sgn(south) != currentCellSign) {
    float distCandidate = - current / (south - current);
    if (distCandidate < dist) {
      dist = distCandidate;
      closestPoint = glm::vec3(i, j + dist, k);
    }
  }
  if (sgn(north) != currentCellSign) {
    float distCandidate = - current / (north - current);
    if (distCandidate < dist) {
      dist = distCandidate;
      closestPoint = glm::vec3(i, j - dist, k);
    }
  }

  if (sgn(down) != currentCellSign) {
    float distCandidate = - current / (down - current);
    if (distCandidate < dist) {
      dist = distCandidate;
      closestPoint = glm::vec3(i, j, k + dist);
    }
  }
  if (sgn(up) != currentCellSign) {
    float distCandidate = - current / (up - current);
    if (distCandidate < dist) {
      dist = distCandidate;
      closestPoint = glm::vec3(i, j, k - dist);
    }
  }

  if (dist != INF) {
    closestPointGrid->set(i, j, k, closestPoint);
  }

  distanceGrid->set(i, j, k, d*currentCellSign);
}

void LevelSet::updateNeighborsFrom(GridCoordinate from) {
  unsigned int i = from.x;
  unsigned int j = from.y;
  unsigned int k = from.z;


  if (i + 1 < w) updateFromCell(from, GridCoordinate(i+1, j, k)); // right
  if (i > 0) updateFromCell(from, GridCoordinate(i-1, j, k)); // left
  if (j + 1 < h) updateFromCell(from, GridCoordinate(i, j+1, k)); // south
  if (j > 0) updateFromCell(from, GridCoordinate(i, j-1, k)); // nort
  if (k + 1 < d) updateFromCell(from, GridCoordinate(i, j, k + 1)); // up
  if (k > 0) updateFromCell(from, GridCoordinate(i, j, k - 1)); // down

}

void LevelSet::updateFromCell(GridCoordinate from,
                              GridCoordinate to) {


  unsigned int xFrom = from.x;
  unsigned int yFrom = from.y;
  unsigned int zFrom = from.z;
  unsigned int xTo = to.x;
  unsigned int yTo = to.y;
  unsigned int zTo = to.z;

  float d = distanceGrid->get(xTo, yTo, zTo);
  glm::vec3 pointCandidate = closestPointGrid->get(xFrom, yFrom, zFrom);
  float dCandidate = glm::distance(pointCandidate, glm::vec3(xTo, yTo, zTo));
  
  if (dCandidate < glm::abs(d)) {
    distanceGrid->set(xTo, yTo, zTo, dCandidate*sgn(d));
    closestPointGrid->set(xTo, yTo, zTo, pointCandidate);
    gridHeap->insert(GridCoordinate(xTo, yTo, zTo));
  }
}

void LevelSet::fastMarch() {
  while (!(gridHeap->empty())) {
    GridCoordinate c = gridHeap->pop();
    if (distanceGrid->get(c) < 5.0) {
      updateNeighborsFrom(GridCoordinate(c.x, c.y, c.z));
    }
  }
}

// void LevelSet::fastSweep() {
//   const float deltaX = 1.0;

//   // columns
//   for(unsigned i = 0; i < w; i++) {
    
//     // sweep down
//     for(unsigned j = 0; j < h; j++) {
//       propagateDistance(0, 1);
//     }

//     // sweep up
//     for(unsigned j = h-1; j >= 0; j--) {
//       propagateDistance(0, -1);
//     }
//   }

//   // rows
//   for(unsigned j = 0; j < h; j++) {
    
//     // sweep right
//     for(unsigned i = 0; i < w; i++) {
//       propagateDistance(1, 0);
//     }
    
//     // sweep left
//     for(unsigned i = w-1; i >= 0; i--) {
//       propagateDistance(-1, 0);
//     }
//   }
// }

// void LevelSet::propagateDistance(int &dx, int &dy) {
//   float deltaX = 1.0*(glm::abs(a) + glm::abs(b));

//   float current = distanceGrid->get(i, j);
//   float predecessor = distanceGrid->safeGet(i + dx, j + dy);
  
//   precedingSign = sgn(predecessor);
//   float candidate = predecessor + deltaX*precedingSign;
  
//   if(glm::abs(candidate) < glm::abs(current)) {
//     distanceGrid->set(i, j, candidate);
//   }
// }

/**
 * Init Level set from analytic function
 * @param sdf Anlytic SignedDistanceFunction
 */
void LevelSet::initializeDistanceGrid(SignedDistanceFunction sdf) {
  distanceGrid->setForEach([&](unsigned int i, unsigned int j, unsigned int k){
      return sdf(i, j, k);
  });
}

void LevelSet::updateCellTypes() {
  cellTypeGrid->setForEach([&](unsigned int i, unsigned int j, unsigned int k){
      if(cellTypeGrid->get(i, j, k) != CellType::SOLID ) {
        if(distanceGrid->get(i, j, k) > 0) {
        return CellType::EMPTY;
      }
      return CellType::FLUID;
    } else {
      return CellType::SOLID;
    }
  });
}

void LevelSet::setCellTypeGrid(Grid<CellType> const* const ctg) {
  cellTypeGrid->setForEach([&](unsigned int i, unsigned int j, unsigned int k){
      return ctg->get(i, j, k);
  });
}

OrdinalGrid<float> const *const LevelSet::getDistanceGrid() const {
  return distanceGrid;
}

Grid<CellType> const *const LevelSet::getCellTypeGrid() const {
  return cellTypeGrid;
}

Grid<bool> const *const LevelSet::getDoneGrid() const{
  return doneGrid;
}

Grid<glm::vec3> const *const LevelSet::getClosestPointGrid() const{
  return closestPointGrid;
}

/**
 * Custom sgn function. Convention (0, INF) outside, (-INF, 0] inside
 * @param  val value
 * @return     inside(-1)/outside(1) fluid
 */
int LevelSet::sgn(float &val) {
  return (0.0f < val) - (0.0f >= val);
}
