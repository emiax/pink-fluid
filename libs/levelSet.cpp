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
  oldDistanceGrid = new OrdinalGrid<float>(w, h, d);
  cellTypeGrid = new Grid<CellType>(w, h, d);
  initSDF = new SignedDistanceFunction(sdf.getFunction());
  
  gridHeap = new GridHeap(w, h, d, distanceGrid);
  closestPointGrid = new Grid<glm::vec3>(w, h, d);

  setCellTypeGrid(ctg);
  initializeDistanceGrid(sdf);
  updateCellTypes();
  targetVolume = currentVolume;
  // std::cout << "targetVolume = " << targetVolume << std::endl;
}

LevelSet::LevelSet(unsigned int w, unsigned int h, unsigned int d, SignedDistFunc sdf, std::function<CellType (unsigned int i, unsigned int j, unsigned int k)> ctg){
  this->w = w;
  this->h = h;
  this->d = d;

  doneGrid = new Grid<bool>(w, h, d);
  distanceGrid = new OrdinalGrid<float>(w, h, d);
  oldDistanceGrid = new OrdinalGrid<float>(w, h, d);
  cellTypeGrid = new Grid<CellType>(w, h, d);
  initSDF = new SignedDistanceFunction(sdf);

  gridHeap = new GridHeap(w,h,d, distanceGrid);
  closestPointGrid = new Grid<glm::vec3>(w, h, d);

  cellTypeGrid->setForEach(ctg);
  initializeDistanceGrid(*initSDF);
  updateCellTypes();
  targetVolume = currentVolume;
  // std::cout << "targetVolume = " << targetVolume << std::endl;
}

LevelSet::LevelSet(const LevelSet& origin) {
  w = origin.w;
  h = origin.h;
  d = origin.d;

  doneGrid = new Grid<bool>(w, h, d);
  cellTypeGrid = new Grid<CellType>(*origin.cellTypeGrid);
  initSDF = new SignedDistanceFunction(origin.initSDF->getFunction());

  distanceGrid = new OrdinalGrid<float>(*origin.distanceGrid);
  oldDistanceGrid = new OrdinalGrid<float>(w, h, d);

  gridHeap = new GridHeap(w,h,d, distanceGrid);
  closestPointGrid = new Grid<glm::vec3>(w, h, d);

  targetVolume = origin.targetVolume;
  currentVolume = origin.currentVolume;
}


LevelSet::~LevelSet() {
  delete doneGrid;
  delete distanceGrid;
  delete cellTypeGrid;
  delete initSDF;
  delete gridHeap;
  delete closestPointGrid;
}


void LevelSet::merge(LevelSet *other) {

  OrdinalGrid<float> *oldSdf = oldDistanceGrid;
  OrdinalGrid<float> *sdf = distanceGrid;
  OrdinalGrid<float> *otherSdf = other->distanceGrid;

  Grid<CellType> *cellTypes = cellTypeGrid;
  Grid<CellType> *otherCellTypes = other->cellTypeGrid;
  
  if (w != other->w || h != other->h || d != other->d) {
    return;
  }

  for(auto k = 0u; k < d; k++){
    for(auto j = 0u; j < h; j++){
      for(auto i = 0u; i < w; i++){
        float aSdf = sdf->get(i, j, k);
        float bSdf = otherSdf->get(i, j, k);

        CellType aType = cellTypes->get(i, j, k);
        CellType bType = otherCellTypes->get(i, j, k);

        sdf->set(i, j, k, std::min(aSdf, bSdf));

        if (aType == CellType::SOLID || bType == CellType::SOLID) {
          cellTypes->set(i, j, k, CellType::SOLID);
          sdf->set(i, j, k, 1000000); // a scientific constant for a shit ton of air.
        } else if (aType == CellType::FLUID || bType == CellType::FLUID) {
          cellTypes->set(i, j, k, CellType::FLUID);
        } else {
          cellTypes->set(i, j, k, CellType::EMPTY);
        }

      }
    }
  }

  for(auto k = 0u; k < d; k++){
    for(auto j = 0u; j < h; j++){
      for(auto i = 0u; i < w; i++){
        float aSdf = distanceGrid->get(i, j, k);
        std::cout << aSdf << std::endl;
      }
    }
  }

}

void LevelSet::reinitialize() {
  std::swap(oldDistanceGrid, distanceGrid);
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
  float current = oldDistanceGrid->get(i, j, k);
  float east = oldDistanceGrid->clampGet(i+1, j, k);
  float west = oldDistanceGrid->clampGet(i-1, j, k);
  float north = oldDistanceGrid->clampGet(i, j+1, k);
  float south = oldDistanceGrid->clampGet(i, j-1, k);
  float down = oldDistanceGrid->clampGet(i, j, k+1);
  float up = oldDistanceGrid->clampGet(i, j, k-1);

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
  if (sgn(north) != currentCellSign) {
    float distCandidate = - current / (north - current);
    if (distCandidate < dist) {
      dist = distCandidate;
      closestPoint = glm::vec3(i, j + dist, k);
    }
  }
  if (sgn(south) != currentCellSign) {
    float distCandidate = - current / (south - current);
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

  distanceGrid->set(i, j, k, dist*currentCellSign);
  
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
    updateNeighborsFrom(GridCoordinate(c.x, c.y, c.z));
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

  // cell type update done - update current volume
  updateCurrentVolume();
}

void LevelSet::updateCurrentVolume() {
  unsigned int totalFluidCells = 0;

  for(unsigned k = 0; k < d; ++k) {
    for(unsigned j = 0; j < h; ++j) {
      for(unsigned i = 0; i < w; ++i) {
        if(cellTypeGrid->get(i, j, k) == CellType::FLUID) {
          ++totalFluidCells; 
        }
      }
    }
  }

  currentVolume = (float)totalFluidCells / (float)(w*h);
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
int LevelSet::sgn(const float &val) {
  return (0.0f < val) - (0.0f >= val);
}

std::ostream& LevelSet::write(std::ostream& stream){
  distanceGrid->write(stream);
  cellTypeGrid->write(stream);
  stream.write(reinterpret_cast<char*>(&w), sizeof(w));
  stream.write(reinterpret_cast<char*>(&h), sizeof(h));
  stream.write(reinterpret_cast<char*>(&d), sizeof(d));

  stream.write(reinterpret_cast<char*>(&targetVolume), sizeof(targetVolume));
  return stream;
}

std::istream& LevelSet::read(std::istream& stream){
  distanceGrid->read(stream);
  cellTypeGrid->read(stream);
  stream.read(reinterpret_cast<char*>(&w), sizeof(w));
  stream.read(reinterpret_cast<char*>(&h), sizeof(h));
  stream.read(reinterpret_cast<char*>(&d), sizeof(d));

  stream.read(reinterpret_cast<char*>(&targetVolume), sizeof(targetVolume));

  reinitialize();
  return stream;
}
