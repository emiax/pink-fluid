#include <simulator.h>
#include <state.h>
#include <ordinalGrid.h>
// #include <exception>
#include <algorithm>
Simulator::Simulator(State *sf, State *st) : stateFrom(sf), stateTo(st) {
  
  w = sf->getW();
  h = sf->getH();

  // TODO: finish check for size mismatch
  if( w != st->getW() || h != st->getH() ) {
    // throw std::exeption();
  }

  // init non-state grids
  divergenceGrid = new OrdinalGrid<float>(w, h);
  pressureGrid = new OrdinalGrid<double>(w, h);
}

Simulator::~Simulator() {}


/**
 * Implicit step function, can only be used if 
 * Simulator is initialized with State constructor
 * @param dt Time step, deltaT
 */
void Simulator::step(float dt) {
  copyBoundaries(stateFrom, stateTo);
  advect(stateFrom, stateTo, dt);
  // calculateDivergence(stateTo, divergenceGrid);

  // swap states
  std::swap(stateFrom, stateTo);
  //State *tempState = stateFrom;
  //stateFrom = stateTo;
  //stateTo = tempState;

}

/**
 * Advects the velocity throughout the grid
 * @param readFrom State to read from, will remain constant
 * @param writeTo State to write to, will be modified after function call
 * @param dt time step length
 */
void Simulator::advect(State const* readFrom, State* writeTo, float dt){
  //X
  for(unsigned int i = 0; i <= w; i++){
    for(unsigned int j = 0; j < h; j++){
      glm::vec2 position = backTrack(readFrom, i, j, dt);
      writeTo->velocityGrid[0]->set(i,j,
        readFrom->velocityGrid[0]->getInterpolated(position)
      );
    }
  }
  //Y
  for(unsigned int i = 0; i < w; i++){
    for(unsigned int j = 0; j <= h; j++){
      glm::vec2 position = backTrack(readFrom, i, j, dt);
      writeTo->velocityGrid[1]->set(i,j,
        readFrom->velocityGrid[1]->getInterpolated(position)
      );
    }
  }
}

void Simulator::copyBoundaries(State const* readFrom, State* writeTo) {
  writeTo->setBoundaryGrid(readFrom->getBoundaryGrid());
}



glm::vec2 Simulator::backTrack(State const* readFrom, int i, int j, float dt){
  glm::vec2 position(i,j);
  glm::vec2 v = glm::vec2(readFrom->velocityGrid[0]->get(i,j), 
                          readFrom->velocityGrid[1]->get(i,j));
  glm::vec2 midPos = position - (dt/2) * v;
  
  glm::vec2 midV = glm::vec2(readFrom->velocityGrid[0]->getInterpolated(midPos), 
                             readFrom->velocityGrid[1]->getInterpolated(midPos));
  return position-dt*midV;
}



void Simulator::calculateDivergence(State const* readFrom, OrdinalGrid<float>* toDivergenceGrid) {
  for(unsigned int i = 0; i < w; i++){
    for(unsigned int j = 0; j < h; j++){
      float entering = readFrom->velocityGrid[0]->get(i, j) + readFrom->velocityGrid[1]->get(i, j);
      float leaving = readFrom->velocityGrid[0]->get(i + 1, j) + readFrom->velocityGrid[1]->get(i, j + 1);
      
      float divergence = leaving - entering;
      toDivergenceGrid->set(i, j, divergence);
    }
  }
}

void Simulator::jacobiIteration(unsigned int nIterations) {
  for(unsigned int i = 0; i < nIterations; ++i) {
    
  }
}

OrdinalGrid<double>* Simulator::resetPressureGrid() {
  for (unsigned int j = 0; j < h; ++j) {
    for (unsigned int i = 0; i < w; ++i) {
      pressureGrid->set(i, j, 0.0);
    }
  }
  return pressureGrid;
}
