#include <state.h>
#include <ordinalGrid.h>



VelocityGrid::VelocityGrid(int w, int h){
    u = new OrdinalGrid<float>(w+1,h);
    v = new OrdinalGrid<float>(w,h+1);
}
VelocityGrid::~VelocityGrid(){
  delete u;
  delete v;
}


State::State(unsigned int width, unsigned int height) : w(width), h(height) {
  velocityGrid = new VelocityGrid(w,h);
  resetVelocityGrids();
}


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
 * Copies velocity grid to internal velocity grid
 */
void State::setVelocityGrid(VelocityGrid *velocity){
  for(unsigned int i = 0u; i < w; i++){
    for(unsigned int j = 0u; j < h; j++){
      velocityGrid->u->set(i, j, velocity->u->get(i,j));
      velocityGrid->v->set(i, j, velocity->v->get(i,j));
    }
  }
}

unsigned int State::getW() {
  return w;
}

unsigned int State::getH() {
  return h;
}

VelocityGrid const *const State::getVelocityGrid() const{
  return velocityGrid;
};

State::~State() {
  delete velocityGrid;
}
