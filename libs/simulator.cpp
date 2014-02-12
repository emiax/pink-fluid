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
  advect(stateFrom, stateTo, dt);
  calculateDivergence(stateTo, divergenceGrid);
  jacobiIteration(40);
  gradientSubtraction(stateTo, dt);

  // swap states
  std::swap(stateFrom, stateTo);
  resetPressureGrid();
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
      writeTo->velocityGrid->u->set(i,j,
                                    readFrom->velocityGrid->u->getInterpolated(position)
      );
    }
  }
  //Y
  for(unsigned int i = 0; i < w; i++){
    for(unsigned int j = 0; j <= h; j++){
      glm::vec2 position = backTrack(readFrom, i, j, dt);
      writeTo->velocityGrid->v->set(i,j,
                                    readFrom->velocityGrid->v->getInterpolated(position)
      );
    }
  }
}


glm::vec2 Simulator::backTrack(State const* readFrom, int i, int j, float dt){
  glm::vec2 position(i,j);
  glm::vec2 v = glm::vec2(readFrom->velocityGrid->u->get(i,j), 
                          readFrom->velocityGrid->v->get(i,j));
  glm::vec2 midPos = position - (dt/2) * v;
  
  glm::vec2 midV = glm::vec2(readFrom->velocityGrid->u->getInterpolated(midPos), 
                             readFrom->velocityGrid->v->getInterpolated(midPos));
  return position-dt*midV;
}



void Simulator::calculateDivergence(State const* readFrom, OrdinalGrid<float>* toDivergenceGrid) {
  for(unsigned int i = 0; i < w; i++){
    for(unsigned int j = 0; j < h; j++){
      float entering = readFrom->velocityGrid->u->get(i, j) + readFrom->velocityGrid->v->get(i, j);
      float leaving = readFrom->velocityGrid->u->get(i + 1, j) + readFrom->velocityGrid->v->get(i, j + 1);
      
      float divergence = leaving - entering;
      toDivergenceGrid->set(i, j, divergence*0.5);
    }
  }
}

void Simulator::jacobiIteration(unsigned int nIterations) {
  
  double pL, pR, pU, pD, p;
  float divergence;
  const float sqDeltaX = 1.0f;

  for(unsigned int k = 0; k < nIterations; ++k) {

    for (unsigned int j = 0; j < h; ++j) {
      for (unsigned int i = 0; i < w; ++i) {
        pL = pressureGrid->getInterpolated(i-1, j);
        pR = pressureGrid->getInterpolated(i+1, j);
        pU = pressureGrid->getInterpolated(i-1, j);
        pD = pressureGrid->getInterpolated(i+1, j);
        divergence = divergenceGrid->get(i, j);

        // discretized poisson equation
        p = pL + pR + pU + pD - sqDeltaX * divergence;
        p *= 0.25;

        pressureGrid->set(i, j, p);
      }
    }

  }
}

void Simulator::gradientSubtraction(State *state, float dt) {
  
  const float deltaX = 1.0f;
  const float density = 1.0f;
  const float scale = dt / (density * deltaX);

  float u, wu, pL, pR;
  OrdinalGrid<float> *uVelocityGrid = state->velocityGrid->u;

  for (unsigned int j = 0; j < h; ++j) {
    for (unsigned int i = 0; i < w; ++i) {
      wu = uVelocityGrid->get(i, j);
      pL = pressureGrid->getInterpolated(i-1, j);
      pR = pressureGrid->get(i, j);
      u = wu - scale * (pR - pL);
      uVelocityGrid->set(i, j, u);
    }
  }

  float v, wv, pU, pD;
  OrdinalGrid<float> *vVelocityGrid = state->velocityGrid->v;

  for (unsigned int j = 0; j < h; ++j) {
    for (unsigned int i = 0; i < w; ++i) {
      wv = vVelocityGrid->get(i, j);
      pU = pressureGrid->getInterpolated(i, j-1);
      pD = pressureGrid->get(i, j);
      v = wv - scale * (pD - pU);
      vVelocityGrid->set(i, j, v);
    }
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
