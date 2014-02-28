#include <state.h>
#include <velocityGrid.h>
#include <ordinalGrid.h>
#include <iostream>
#include <levelSet.h>



/**
 * Constructor.
 */
State::State(unsigned int width, unsigned int height) : w(width), h(height) {
  velocityGrid = new VelocityGrid(w,h);
  inkGrid = new OrdinalGrid<glm::vec3>(w, h);

  resetVelocityGrids();
}

/**
 * Destructor.
 */
State::~State() {
  delete velocityGrid;
  delete inkGrid;
}


/**
 * Reset velocity grids
 */
void State::resetVelocityGrids() {
  for(unsigned int i = 0u; i <= w; i++){
    for(unsigned int j = 0u; j < h; j++){
      velocityGrid->u->set(i, j, 0.0f);
    }
  }
  for(unsigned int i = 0u; i < w; i++){
    for(unsigned int j = 0u; j <= h; j++){
      velocityGrid->v->set(i, j, 0.0f);
    }
  }
}


/**
 * Copy velocity grid to internal velocity grid
 */
void State::setVelocityGrid(VelocityGrid const* const velocity){
  for(unsigned int i = 0u; i < w; i++){
    for(unsigned int j = 0u; j <= h; j++){
      velocityGrid->v->set(i, j, velocity->v->get(i,j));
    }
  }

  for(unsigned int i = 0u; i <= w; i++){
    for(unsigned int j = 0u; j < h; j++){
      velocityGrid->u->set(i, j, velocity->u->get(i,j));
    }
  }
}


/**
 * Get width
 */
unsigned int State::getW() {
  return w;
}


/**
 * Get height
 */
unsigned int State::getH() {
  return h;
}


/**
 * Set cell type grid
 */
void State::setCellTypeGrid(Grid<CellType>const* const ctg) {
  levelSet->setCellTypeGrid(ctg);
}

/**
 * Set ink grid
 * @param ink grid to copy concentration values from
 */
void State::setInkGrid(OrdinalGrid<glm::vec3> const* const ink) {
  for (unsigned int j = 0; j < h; ++j) {
    for (unsigned int i = 0; i < w; ++i) {
      this->inkGrid->set( i, j, ink->get(i, j) );
    }
  }
}

/**
 * Set signed distance grid 
 * @param sdg grid to copy signed distance from
 */
// void State::setSignedDistanceGrid(OrdinalGrid<float> const* const sdg) {
//   for (unsigned int j = 0; j < h; ++j) {
//     for (unsigned int i = 0; i < w; ++i) {
//       this->signedDistanceGrid->set( i, j, sdg->get(i, j) );
//     }
//   }
// }

/**
 * Set level set
 * @param levelSet LevelSet object
 */
void State::setLevelSet(LevelSet *ls) {
  levelSet = new LevelSet( w, h, *(ls->initSDF), ls->getCellTypeGrid() );
}

/**
 * Get velocity grid 
 */
VelocityGrid const *const State::getVelocityGrid() const{
  return velocityGrid;
};

/**
 * Get cell type grid
 */
Grid<CellType> const *const State::getCellTypeGrid() const {
  return levelSet->getCellTypeGrid();
}

/**
 * Get ink grid
 * @return const pointer to ink grid.
 */
OrdinalGrid<glm::vec3> const *const State::getInkGrid() const {
  return inkGrid;
}

/**
 * Get signed distance grid
 * @return const pointer to signed distance grid.
 */
OrdinalGrid<float> const *const State::getSignedDistanceGrid() const {
  return levelSet->getDistanceGrid();
}


/**
 * Get closest point grid
 * @return const pointer to closest point grid.
 */
Grid<glm::vec2> const *const State::getClosestPointGrid() const {
  return levelSet->getClosestPointGrid();
}

