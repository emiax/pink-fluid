#include <simulator.h>
#include <state.h>
#include <ordinalGrid.h>
#include <exception>

Simulator::Simulator(State *sPing, State *sPong) : stateFrom(sPing), stateTo(sPong) {
  
  w = sPing->getW();
  h = sPing->getH();

  // TODO: check for size mismatch
  if( w != sPong->getW() || h != sPong->getH() ) {
    // throw std::exeption();
  }
}

Simulator::~Simulator() {}


/**
 * Implicit step function, can only be used if 
 * Simulator is initialized with State constructor
 * @param dt Time step, deltaT
 */
void Simulator::step(float dt) {
  advect(stateFrom, stateTo, dt);
  calculateDivergence(stateTo, divergenceGrid);
  

  // swap states
  State *tempState = stateFrom;
  stateFrom = stateTo;
  stateTo = tempState;

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
