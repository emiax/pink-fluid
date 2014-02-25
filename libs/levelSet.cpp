#include <levelSet.h>


LevelSet::LevelSet(unsigned int w, unsigned int h, SignedDistanceFunction sdf){
  this->w = w;
  this->h = h;
  doneGrid = new OrdinalGrid<bool>(w,h);
  distanceGrid = new OrdinalGrid<float>(w,h);

  initializeDistanceGrid(sdf);
}

void LevelSet::reinitialize(LevelSet *const levelSetFrom) {
  markClosestAsDone(levelSetFrom);
  fastSweep(levelSetFrom);
}

void LevelSet::markClosestAsDone(LevelSet *const levelSetFrom){
  for(auto i = 0u; i < w; i++){
    for(auto j = 0u; j < h; j++){
      //Currently only creates the levelset-based on fluid
      BoundaryType currentCellType = levelSetFrom->cellTypeGrid->get(i,j);
      if(currentCellType == BoundaryType::FLUID){
        if(levelSetFrom->cellTypeGrid->safeGet(i+1,j) == BoundaryType::EMPTY){
          doneGrid->set(i+1,j, true);
          doneGrid->set(i,j, true);
        }
        else if(levelSetFrom->cellTypeGrid->safeGet(i-1,j) == BoundaryType::EMPTY){
          doneGrid->set(i-1,j, true);
          doneGrid->set(i,j, true);
        }
        else if(levelSetFrom->cellTypeGrid->safeGet(i,j+1) == BoundaryType::EMPTY){
          doneGrid->set(i,j+1, true);
          doneGrid->set(i,j, true);
        }
        else if(levelSetFrom->cellTypeGrid->safeGet(i,j-1) == BoundaryType::EMPTY){
          doneGrid->set(i,j-1, true);
          doneGrid->set(i,j, true);
        }
        else{
          doneGrid->set(i,j, false);
        }
      }
    }
  }
}

void LevelSet::fastSweep(LevelSet *const levelSetFrom) {
  
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

Grid<bool> const *const LevelSet::getDoneGrid() const{
  return doneGrid;
}
