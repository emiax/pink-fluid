#include <levelSet.h>


LevelSet::LevelSet(unsigned int w, unsigned int h){
  distanceGrid = new OrdinalGrid<glm::vec2>(w,h);
  doneGrid = new OrdinalGrid<bool>(w,h);
  this->w = w;
  this->h = h;
}

/**
 * Intializes a state based on data in BoundaryGrid.
 * This grid can then be advected by advectLevelSet
 * @param state State to create the level set on. 
 */
void LevelSet::initializeLevelSet(Grid<BoundaryType> const *const boundary){
  for(auto i = 0u; i < w; i++){
    for(auto j = 0u; j < h; j++){
      //Currently only creates the levelset-based on fluid
      BoundaryType currentCellType = boundary->get(i,j);
      if(boundary->safeGet(i+1,j) != currentCellType){
        distanceGrid->set(i,j, glm::vec2(i+0.5, j));
        doneGrid->set(i,j, true);
      }
      else if(boundary->safeGet(i-1,j) != currentCellType){
        distanceGrid->set(i,j, glm::vec2(i-0.5, j));
        doneGrid->set(i,j, true);
      }
      else if(boundary->safeGet(i,j+1) != currentCellType){
        distanceGrid->set(i,j, glm::vec2(0.5, j+0.5));
        doneGrid->set(i,j, true);
      }
      else if(boundary->safeGet(i,j-1) != currentCellType){
        distanceGrid->set(i,j, glm::vec2(i, j-0.5));
        doneGrid->set(i,j, true);
      }
      else{
        doneGrid->set(i,j, false);
      }
    }
  }
}


Grid<bool> const *const LevelSet::getDoneGrid(){
  return doneGrid;
}
