#include <state.h>
#include <ordinalGrid.h>

State::State(unsigned int width, unsigned int height) : w(width), h(height) {
  velocityGrid = new OrdinalGrid<float>*[2];
  velocityGrid[0] = new OrdinalGrid<float>(w+1,h);
  velocityGrid[1] = new OrdinalGrid<float>(w,h+1);
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
 * Copies velocity grid to internal velocity grid
 */
void State::setVelocityGrid(OrdinalGrid<float>** velocity){
  for(unsigned int i = 0u; i < w; i++){
    for(unsigned int j = 0u; j < h; j++){
      velocityGrid[0]->set(i, j, velocity[0]->get(i,j));
      velocityGrid[1]->set(i, j, velocity[1]->get(i,j));
    }
  }
}

unsigned int State::getW() {
  return w;
}

unsigned int State::getH() {
  return h;
}

OrdinalGrid<float>const *const *const State::getVelocityGrid() const{
  return velocityGrid;
};

State::~State() {
  delete velocityGrid[0];
  delete velocityGrid[1];
  delete[] velocityGrid;
}
