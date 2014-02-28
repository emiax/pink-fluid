#include <levelSet.h>
#include <glm/ext.hpp>
#include <gridHeap.h>
#include <iostream>

LevelSet::LevelSet(unsigned int w, unsigned int h, SignedDistanceFunction sdf, Grid<CellType> const* const ctg){
  this->w = w;
  this->h = h;
  

  doneGrid = new OrdinalGrid<bool>(w,h);
  distanceGrid = new OrdinalGrid<float>(w,h);
  cellTypeGrid = new Grid<CellType>(w, h);
  this->initSDF = new SignedDistanceFunction(sdf.getFunction());

  gridHeap = new GridHeap(w, h, distanceGrid);
  closestPointGrid = new Grid<glm::vec2>(w, h);

  setCellTypeGrid(ctg);
  initializeDistanceGrid(sdf);
  updateCellTypes();
}


// TODO: This needs to be implemented when LevelSet functionality is completed
/*LevelSet::LevelSet(unsigned int w, unsigned int h, Grid<CellType> const* const ctg) {
  this->LevelSet(w,h [&](unsigned int i, unsigned int j){
    if()
  })
  this->w = w;
  this->h = h;
  doneGrid = new OrdinalGrid<bool>(w,h);
  distanceGrid = new OrdinalGrid<float>(w,h);
  cellTypeGrid = new Grid<CellType>(w, h);


  setCellTypeGrid(ctg);
  markClosestAsDone();
}*/

void LevelSet::reinitialize() {
  updateInterfaceNeighbors();
  fastMarch();
  updateCellTypes();
}

void LevelSet::updateInterfaceNeighbors(){
  for(auto i = 0u; i < w; i++){
    for(auto j = 0u; j < h; j++){
      //Currently only creates the levelset-based on fluid
      CellType currentCellType = cellTypeGrid->get(i,j);
      updateInterfaceNeighborCell(i, j);
    }
  }

  for(auto i = 0u; i < w; i++){
    for(auto j = 0u; j < h; j++){
      if (glm::abs(distanceGrid->get(i, j)) < INF) {
        updateNeighborsFrom(i, j);
      }
    }
  }
}

void LevelSet::updateInterfaceNeighborCell(unsigned int i, unsigned int j) {
  float current = distanceGrid->get(i,j);
  float right = distanceGrid->clampGet(i+1,j);
  float left = distanceGrid->clampGet(i-1,j);
  float down = distanceGrid->clampGet(i,j+1);
  float up = distanceGrid->clampGet(i,j-1);
  int currentCellSign = sgn(current);
  
  float d = INF;
  glm::vec2 closestPoint(i, j);

  
  if (sgn(right) != currentCellSign) {
    float dCandidate = - current / (right - current);
    if (dCandidate < d) {
      d = dCandidate;
      closestPoint = glm::vec2(i + d, j);
    }
  }
  if (sgn(left) != currentCellSign) {
    float dCandidate = - current / (left - current);
    if (dCandidate < d) {
      d = dCandidate;
      closestPoint = glm::vec2(i - d, j);
    }
  }
  if (sgn(down) != currentCellSign) {
    float dCandidate = - current / (down - current);
    if (dCandidate < d) {
      d = dCandidate;
      closestPoint = glm::vec2(i, j + d);
    }
  }
  if (sgn(up) != currentCellSign) {
    float dCandidate = - current / (up - current);
    if (dCandidate < d) {
      d = dCandidate;
      closestPoint = glm::vec2(i, j - d);
    }
  }

  if (d != INF) {
    closestPointGrid->set(i, j, closestPoint);
  }

  distanceGrid->set(i, j, d*currentCellSign);
}

void LevelSet::updateNeighborsFrom(unsigned int i, unsigned int j) {

  float current = distanceGrid->get(i,j);

  if (i + 1 < w) updateFromCell(i, j, i+1, j); // right
  if (i > 0) updateFromCell(i, j, i-1, j); // left
  if (j + 1 < h) updateFromCell(i, j, i, j+1); // down
  if (j > 0) updateFromCell(i, j, i, j-1); // up
}

void LevelSet::updateFromCell(unsigned int xFrom,
                              unsigned int yFrom,
                              unsigned int xTo,
                              unsigned int yTo) {

  float d = distanceGrid->get(xTo, yTo);
  glm::vec2 pointCandidate = closestPointGrid->get(xFrom, yFrom);
  float dCandidate = glm::distance(pointCandidate, glm::vec2(xTo, yTo));
  
  if (dCandidate < glm::abs(d)) {
    distanceGrid->set(xTo, yTo, dCandidate*sgn(d));
    closestPointGrid->set(xTo, yTo, pointCandidate);
    gridHeap->insert(GridCoordinate(xTo, yTo));
  }
}

void LevelSet::fastMarch() {
  while (!(gridHeap->empty())) {
    GridCoordinate c = gridHeap->pop();
    updateNeighborsFrom(c.x, c.y);
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
  distanceGrid->setForEach([&](unsigned int i, unsigned int j){
    return sdf(i, j);
  });
}

void LevelSet::updateCellTypes() {
  cellTypeGrid->setForEach([&](unsigned int i, unsigned int j){
    if(cellTypeGrid->get(i, j) != CellType::SOLID ) {
      if(distanceGrid->get(i, j) > 0) {
        return CellType::EMPTY;
      }
      return CellType::FLUID;
    } else {
      return CellType::SOLID;
    }
  });
}

void LevelSet::setCellTypeGrid(Grid<CellType> const* const ctg) {
  cellTypeGrid->setForEach([&](unsigned int i, unsigned int j){
    return ctg->get(i, j);
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

Grid<glm::vec2> const *const LevelSet::getClosestPointGrid() const{
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
