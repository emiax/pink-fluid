#include <state.h>
#include <velocityGrid.h>
#include <ordinalGrid.h>
#include <iostream>




/**
 * Constructor.
 */
State::State(unsigned int width, unsigned int height) : w(width), h(height) {
  velocityGrid = new VelocityGrid(w,h);
  boundaryGrid = new Grid<BoundaryType>(w, h);
  inkGrid = new OrdinalGrid<glm::vec3>(w, h);

  resetVelocityGrids();
}

/**
 * Destructor.
 */
State::~State() {
  delete velocityGrid;
  delete boundaryGrid;
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
 * Set boundary grid
 */
void State::setBoundaryGrid(Grid<BoundaryType>const* const boundary) {
  for(unsigned int i = 0u; i < w; i++){
    for(unsigned int j = 0u; j < h; j++){
      this->boundaryGrid->set(i, j, boundary->get(i, j));
    }
  }
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

// OrdinalGrid<double>const *const State::getPressureGrid() const{
//   return pressureGrid;
// };

/**
 * Get velocity grid 
 */
VelocityGrid const *const State::getVelocityGrid() const{
  return velocityGrid;
};

/**
 * Get boundary grid
 */
Grid<BoundaryType>const *const State::getBoundaryGrid() const {
  return boundaryGrid;
}

/**
 * Get ink grid
 * @return const pointer to ink grid.
 */
OrdinalGrid<glm::vec3> const *const State::getInkGrid() const {
  return inkGrid;
}


