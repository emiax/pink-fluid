#include <state.h>
#include <ordinalGrid.h>
#include <iostream>

State::State(unsigned int width, unsigned int height) : w(width), h(height) {
  velocityGrid = new OrdinalGrid<float>*[2];
  velocityGrid[0] = new OrdinalGrid<float>(w + 1, h);
  velocityGrid[1] = new OrdinalGrid<float>(w, h + 1);
  fluidGrid = new Grid<bool>(w, h);
  boundaryGrid = new Grid<bool>(w, h);
  resetVelocityGrids();

}


void State::resetVelocityGrids() {
  for(unsigned int i = 0u; i <= w; i++){
    for(unsigned int j = 0u; j < h; j++){
      velocityGrid[0]->set(i, j, 0.0f);
    }
  }
  for(unsigned int i = 0u; i < w; i++){
    for(unsigned int j = 0u; j <= h; j++){
      velocityGrid[1]->set(i, j, 0.0f);
    }
  }
}

/**
 * Copy velocity grid to internal velocity grid
 */
void State::setVelocityGrid(OrdinalGrid<float>const* const* velocity){
  for(unsigned int i = 0u; i < w; i++){
    for(unsigned int j = 0u; j < h; j++){
      velocityGrid[0]->set(i, j, velocity[0]->get(i,j));
      velocityGrid[1]->set(i, j, velocity[1]->get(i,j));
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
void State::setBoundaryGrid(Grid<bool>const* const boundary) {
  for(unsigned int i = 0u; i < w; i++){
    for(unsigned int j = 0u; j < h; j++){
      this->boundaryGrid->set(i, j, boundary->get(i, j));
    }
  }
}


/**
 * Set fluid grid
 */
void State::setFluidGrid(Grid<bool>const* const fluid) {
  for(unsigned int i = 0u; i < w; i++){
    for(unsigned int j = 0u; j < h; j++){
      this->fluidGrid->set(i, j, fluid->get(i, j));
    }
  }
}


// OrdinalGrid<double>const *const State::getPressureGrid() const{
//   return pressureGrid;
// };

/**
 * Get velocity grid 
 */

OrdinalGrid<float>const *const *const State::getVelocityGrid() const{
  return velocityGrid;
};


/**
 * Get boundary grid
 */
Grid<bool>const *const State::getBoundaryGrid() const {
  return boundaryGrid;
}


State::~State() {
  delete velocityGrid[0];
  delete velocityGrid[1];
  delete[] velocityGrid;
}
