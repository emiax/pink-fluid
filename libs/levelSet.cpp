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
  targetVolume = currentVolume;
}

void LevelSet::reinitialize() {
  gridHeap->clear();
  updateInterfaceNeighbors();
  fastMarch();
  updateCellTypes();
  clampInfiniteCells();
  // std::cout << "magicPeople = " << targetVolume << std::endl;
  // std::cout << "voodooPeople = " << currentVolume << std::endl;
}

void LevelSet::updateInterfaceNeighbors(){

  for(unsigned j = 0; j < h; ++j) {
    for(unsigned i = 0; i < w; ++i) {
      //Currently only creates the levelset-based on fluid
      CellType currentCellType = cellTypeGrid->get(i,j);
      updateInterfaceNeighborCell(i, j);
    }
  }

  for(unsigned j = 0; j < h; ++j) {
    for(unsigned i = 0; i < w; ++i) {
      if (glm::abs(distanceGrid->get(i, j)) < INF) {
        updateNeighborsFrom(i, j);
      }
    }
  }
}


void LevelSet::clampInfiniteCells() {
  for(auto j = 0u; j < h; j++) {
    for(auto i = 0u; i < w; i++) {
      if (distanceGrid->get(i, j) > 5.0) {
        distanceGrid->set(i, j, 5.0);
      }
      if (distanceGrid->get(i, j) < -5.0) {
        distanceGrid->set(i, j, -5.0);
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
    // if (fabs(distanceGrid->get(c)) < 5.0) {
      updateNeighborsFrom(c.x, c.y);
    // }
  }
}

float LevelSet::getVolumeError() {
  return targetVolume - currentVolume;
}

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
  unsigned int totalFluidCells = 0;
  
  cellTypeGrid->setForEach([&](unsigned int i, unsigned int j){
    if(cellTypeGrid->get(i, j) != CellType::SOLID ) {
      if(distanceGrid->get(i, j) > 0) {
        return CellType::EMPTY;
      }
      ++totalFluidCells;
      return CellType::FLUID;
    } else {
      return CellType::SOLID;
    }
  });

  currentVolume = (float)totalFluidCells / (float)(w*h);
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
