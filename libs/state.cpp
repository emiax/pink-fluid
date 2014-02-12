#include <state.h>
#include <ordinalGrid.h>

State::State(unsigned int width, unsigned int height) : w(width), h(height) {
  pressureGrid = new OrdinalGrid<double>(w,h);
  velocityGrid = new OrdinalGrid<float>*[2];
  velocityGrid[0] = new OrdinalGrid<float>(w,h);
  velocityGrid[1] = new OrdinalGrid<float>(w,h);
}

/**
 * Copies pressure grid to internal pressure grid
 *
 */
void State::setPressureGrid(OrdinalGrid<double> *pressure){
  for(unsigned int i = 0u; i < w; i++){
    for(unsigned int j = 0u; h < h; j++){
      pressureGrid->set(i, j, pressure->get(i,j));
    }
  }
}
/**
 * Copies velocity grid to internal velocity grid
 *
 */
void State::setVelocityGrid(OrdinalGrid<float>** velocity){
  velocityGrid = velocity;
  for(unsigned int i = 0u; i < w; i++){
    for(unsigned int j = 0u; h < h; j++){
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

OrdinalGrid<double>const *const State::getPressureGrid() const{
  return pressureGrid;
};

OrdinalGrid<float>const *const *const State::getVelocityGrid() const{
  return velocityGrid;
};

State::~State() {
  delete pressureGrid;
  delete velocityGrid[0];
  delete velocityGrid[1];
  delete[] velocityGrid;
}
