#include <state.h>
#include <ordinalGrid.h>

State::State(unsigned int width, unsigned int height) : w(width), h(height) {
  // pressureGrid = new OrdinalGrid<double>(w,h);
  velocityGrid = new OrdinalGrid<float>*[2];
  velocityGrid[0] = new OrdinalGrid<float>(w + 1, h);
  velocityGrid[1] = new OrdinalGrid<float>(w, h + 1);
  fluidGrid = new Grid<bool>(w, h);
  boundaryGrid = new Grid<bool>(w, h);
}

/**
 * Copies pressure grid to internal pressure grid
 *
 */
// void State::setPressureGrid(OrdinalGrid<double> *pressure){
//   for(unsigned int i = 0u; i < w; i++){
//     for(unsigned int j = 0u; h < h; j++){
//       pressureGrid->set(i, j, pressure->get(i,j));
//     }
//   }
// }

/**
 * Copy velocity grid to internal velocity grid
 *
 */
void State::setVelocityGrid(OrdinalGrid<float>** velocity){
  for(unsigned int i = 0u; i < w; i++){
    for(unsigned int j = 0u; h < h; j++){
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
void State::setBoundaryGrid(Grid<bool>* boundary) {
  for(unsigned int i = 0u; i < w; i++){
    for(unsigned int j = 0u; h < h; j++){
      this->boundaryGrid->set(i, j, boundary->get(i, j));
    }
  }
}


/**
 * Set fluid grid
 */
void State::setFluidGrid(Grid<bool>* fluid) {
  for(unsigned int i = 0u; i < w; i++){
    for(unsigned int j = 0u; h < h; j++){
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

State::~State() {
  // delete pressureGrid;
  delete velocityGrid[0];
  delete velocityGrid[1];
  delete[] velocityGrid;
}
