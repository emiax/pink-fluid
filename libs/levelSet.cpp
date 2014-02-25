#include <levelSet.h>
#include <glm/ext.hpp>

LevelSet::LevelSet(unsigned int w, unsigned int h, SignedDistanceFunction sdf, Grid<CellType> *const const ctg){
  this->w = w;
  this->h = h;
  doneGrid = new OrdinalGrid<bool>(w,h);
  distanceGrid = new OrdinalGrid<float>(w,h);
  cellTypeGrid = new Grid<CellType>(w, h);

  setCellTypeGrid(ctg);
  initializeDistanceGrid(sdf);
  updateCellTypes();
}


// TODO: This needs to be implemented when LevelSet functionality is completed
/*LevelSet::LevelSet(unsigned int w, unsigned int h, Grid<CellType> *const const ctg) {
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
  markClosestAsDone();
  fastSweep();
  updateCellTypes();
}

void LevelSet::markClosestAsDone(){
  for(auto i = 0u; i < w; i++){
    for(auto j = 0u; j < h; j++){
      //Currently only creates the levelset-based on fluid
      CellType currentCellType = levelSetFrom->cellTypeGrid->get(i,j);
      if(currentCellType != CellType::SOLID){
        float currentCellSign = glm::sign(levelSetFrom->distanceGrid->get(i,j));
        if(glm::sign(levelSetFrom->distanceGrid->safeGet(i+1,j)) != currentCellSign){
          doneGrid->set(i,j, true);
        }
        else if(glm::sign(levelSetFrom->distanceGrid->safeGet(i-1,j)) != currentCellSign){
          doneGrid->set(i,j, true);
        }
        else if(glm::sign(levelSetFrom->distanceGrid->safeGet(i,j+1)) != currentCellSign){
          doneGrid->set(i,j, true);
        }
        else if(glm::sign(levelSetFrom->distanceGrid->safeGet(i,j-1)) != currentCellSign){
          doneGrid->set(i,j, true);
        }
        else{
          doneGrid->set(i,j, false);
        }
      }
    }
  }
}

void LevelSet::fastSweep() {
  bool foundDone = false;
  const float deltaX = 1.0;
  for(unsigned i = 0; i < w; i++) {
    for(unsigned j = 0; j < h; j++) {
      if(!foundDone) continue;
      if(doneGrid->get(i, j) != true) {
        float predecessor = distanceGrid->get(i, j-1);
        int distanceStep = glm::sign(predecessor) ? glm::sign(predecessor) : -1.0;
        distanceStep *= deltaX;
        distanceGrid->set( i, j, predecessor + distanceStep );
      } else {
        foundDone = true;
      }
    }
    foundDone = false;
    for(unsigned j = h-1; j >= 0; j--) {

    }
    foundDone = false;
  }
  for(unsigned j = 0; j < h; j++) {
    for(unsigned i = 0; i < w; i++) {
      
    }
    foundDone = false;
    for(unsigned i = w-1; i >= 0; i--) {

    }
    foundDone = false;
  }
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

void LevelSet::initializeDistanceGrid() {
  markClosestAsDone();
  distanceGrid->setForEach([&](unsigned int i, unsigned int j) {
    if(cellTypeGrid->get(i, j))
  });
}

void LevelSet::updateCellTypes() {
  cellTypeGrid->setForEach([&](unsigned int i, unsigned int j){
    if(cellTypeGrid->get(i, j) != CellType::SOLID ) {
      if(distanceGrid->get(i, j) > 0) {
        return CellType::EMPTY;
      }
      return CellType::FLUID;
    }
  });
}

void LevelSet::setCellTypeGrid(Grid<CellType> *const const ctg) {
  cellTypeGrid->setForEach([&](unsigned int i, unsigned int j){
    return ctg->get(i, j);
  });
}

Grid<bool> const *const LevelSet::getDoneGrid() const{
  return doneGrid;
}
