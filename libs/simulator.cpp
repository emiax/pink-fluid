#include <simulator.h>
#include <state.h>
#include <ordinalGrid.h>
#include <velocityGrid.h>
// #include <exception>
#include <algorithm>

Simulator::Simulator(State *sf, State *st, float scale) : stateFrom(sf), stateTo(st), gridSize(scale){
  
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

/**
 * Destructor.
 */
Simulator::~Simulator() {}


/**
 * Implicit step function, can only be used if
 * Simulator is initialized with State constructor
 * @param dt Time step, deltaT
 */
void Simulator::step(float dt) {
  
  // OrdinalGrid<float> *divergenceOut = new OrdinalGrid<float>(w, h);

  copyBoundaries(stateFrom, stateTo);
  advect(stateFrom, stateTo, dt);
  calculateDivergence(stateTo, divergenceGrid);
  jacobiIteration(stateTo, 100);
  gradientSubtraction(stateTo, dt);

  // calculateDivergence(stateTo, divergenceOut);

  // divergence sum
  // float sumDivIn = 0;
  // float sumDivOut = 0;
  // for (unsigned int j = 0; j < h; ++j) {
  //   for (unsigned int i = 0; i < w; ++i) {
  //     sumDivIn += fabs( divergenceGrid->get(i, j) );
  //     sumDivOut += fabs( divergenceOut->get(i, j) );
  //   }
  // }
  // std::cout << "avg. div in: " << sumDivIn / (w*h) << std::endl;
  // std::cout << "avg. div out: " << sumDivOut / (w*h) << std::endl;
  // std::cout << std::endl;

  // std::cin.get();

  // Variable time step calculation
  deltaT = calculateDeltaT(maxVelocity(stateTo->velocityGrid), glm::vec2(0));
  // swap states
  std::swap(stateFrom, stateTo);
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
      glm::vec2 position = backTrackU(readFrom, i, j, dt);
      writeTo->velocityGrid->u->set(i,j,
        readFrom->velocityGrid->u->getInterpolated(position)
      );
    }
  }
  //Y
  for(unsigned int i = 0; i < w; i++){
    for(unsigned int j = 0; j <= h; j++){
      glm::vec2 position = backTrackV(readFrom, i, j, dt);
      writeTo->velocityGrid->v->set(i,j,
        readFrom->velocityGrid->v->getInterpolated(position)
      );
    }
  }

  // ink grid
  for (unsigned int j = 0; j < h; ++j) {
    for (unsigned int i = 0; i < w; ++i) {
      glm::vec2 position = backTrackMid(readFrom, i, j, dt);
      writeTo->inkGrid->set(i,j,
        readFrom->inkGrid->getInterpolated(position)
      );
    }
  }

}

/**
 * Copy boundaries
 * @param readFrom State to read from
 * @param writeTo State to write to
 */
void Simulator::copyBoundaries(State const* readFrom, State* writeTo) {
  writeTo->setBoundaryGrid(readFrom->getBoundaryGrid());
}


/**
 * Find the previous position of the temporary particle in the grid that travelled to i, j.
 * @param readFrom State to read from
 * @param i x coordinate
 * @param j y coordinate
 * @param dt the time between the two steps
 * @return          position to copy new value from
 */
glm::vec2 Simulator::backTrackU(State const* readFrom, int i, int j, float dt){
  glm::vec2 position(i,j);
  glm::vec2 v = glm::vec2(readFrom->velocityGrid->u->get(i,j),
                          readFrom->velocityGrid->v->getInterpolated(i-0.5,j+0.5));
  glm::vec2 midPos = position - (dt/2) * v;

  glm::vec2 midV = glm::vec2(readFrom->velocityGrid->u->getInterpolated(midPos),
                             readFrom->velocityGrid->v->getInterpolated(midPos));
  return position-dt*midV;
}

glm::vec2 Simulator::backTrackV(State const* readFrom, int i, int j, float dt){
  glm::vec2 position(i,j);
  glm::vec2 v = glm::vec2(readFrom->velocityGrid->u->get(i+0.5,j-0.5),
                          readFrom->velocityGrid->v->getInterpolated(i,j));
  glm::vec2 midPos = position - (dt/2) * v;

  glm::vec2 midV = glm::vec2(readFrom->velocityGrid->u->getInterpolated(midPos),
                             readFrom->velocityGrid->v->getInterpolated(midPos));
  return position-dt*midV;
}

glm::vec2 Simulator::backTrackMid(State const* readFrom, int i, int j, float dt){
  glm::vec2 position(i,j);
  glm::vec2 v = glm::vec2(readFrom->velocityGrid->u->getInterpolated(i+0.5,j),
                          readFrom->velocityGrid->v->getInterpolated(i,j+0.5));
  glm::vec2 midPos = position - (dt/2) * v;

  glm::vec2 midV = glm::vec2(readFrom->velocityGrid->u->getInterpolated(midPos),
                             readFrom->velocityGrid->v->getInterpolated(midPos));
  return position-dt*midV;
}

/**
 * Calculate divergence.
 * @param readFrom State to read from
 * @param toDivergenceGrid An ordinal grid of floats to write the divergences to
 */
void Simulator::calculateDivergence(State const* readFrom, OrdinalGrid<float>* toDivergenceGrid) {
  OrdinalGrid<float> *u = readFrom->velocityGrid->u;
  OrdinalGrid<float> *v = readFrom->velocityGrid->v;

  for(unsigned int i = 0; i < w; i++){
    for(unsigned int j = 0; j < h; j++){
      float entering = u->get(i, j) + v->get(i, j);
      float leaving = u->get(i + 1, j) + v->get(i, j + 1);

      float divergence = leaving - entering;

      toDivergenceGrid->set(i, j, divergence);
    }
  }
}


/**
 * Perform nIterantions Jacobi iterations in order to calculate pressures.
 * @param readFrom The state to read from
 * @param nIterations number of iterations
 */
void Simulator::jacobiIteration(State const* readFrom, unsigned int nIterations) {

  const float sqDeltaX = 1.0f;
  Grid<bool> const* const boundaryGrid = readFrom->getBoundaryGrid();
  
  resetPressureGrid();

  for(unsigned int k = 0; k < nIterations; ++k) {
    for (unsigned int j = 0; j < h; ++j) {
      for (unsigned int i = 0; i < w; ++i) {

        float divergence;
        int neighbouringFluidCells = 0;

        // is current cell solid?
        if (boundaryGrid->get(i, j)) {
          continue;
        }

        double pL = 0;
        if (!boundaryGrid->get(i - 1, j)) {
          pL = pressureGrid->get(i - 1, j);
          neighbouringFluidCells++;
        } 
        double pR = 0;
        if (!boundaryGrid->get(i + 1, j)) {
          pR = pressureGrid->get(i + 1, j);
          neighbouringFluidCells++;
        } 
        double pU = 0;
        if (!boundaryGrid->get(i, j - 1)) {
          pU = pressureGrid->get(i, j - 1);
          neighbouringFluidCells++;
        } 
        double pD = 0;
        if (!boundaryGrid->get(i, j + 1)) {
          pD = pressureGrid->get(i, j + 1);
          neighbouringFluidCells++;
        } 

        divergence = divergenceGrid->get(i, j);

        // discretized poisson equation
        double p = pL + pR + pU + pD - sqDeltaX * divergence;
        p /= (double) neighbouringFluidCells;

        pressureGrid->set(i, j, p);
      }
    }

  }
}

/**
 * TODO: should iterate pressure cells and update neighbouring
 * velocity values instead of vice-versa.
 * 
 * Subtract pressure gradients from velocity grid
 * @param state State to work on
 * @param dt, The time step
 */
void Simulator::gradientSubtraction(State *state, float dt) {

  const float deltaX = 1.0f;
  const float density = 1.0f;
  const float scale = dt / (density * deltaX);

  OrdinalGrid<float> *uVelocityGrid = state->velocityGrid->u;
  OrdinalGrid<float> *vVelocityGrid = state->velocityGrid->v;

  // looping through pressure cells
  for (unsigned int j = 0; j < h; ++j) {
    for (unsigned int i = 0; i < w; ++i) {

      float uL = uVelocityGrid->get(i, j);
      float uR = uVelocityGrid->get(i+1, j);

      float vU = vVelocityGrid->get(i, j);
      float vD = vVelocityGrid->get(i, j+1);

      float p = pressureGrid->get(i, j);

      uL -= scale * p;
      uR += scale * p;
      vU -= scale * p;
      vD += scale * p;

      uVelocityGrid->set(i, j, uL);
      uVelocityGrid->set(i+1, j, uR);
      vVelocityGrid->set(i, j, vU);
      vVelocityGrid->set(i, j+1, vD);
    }
  }
}

/**
 * Reset pressure grid
 */
OrdinalGrid<double>* Simulator::resetPressureGrid() {
  for (unsigned int j = 0; j < h; ++j) {
    for (unsigned int i = 0; i < w; ++i) {
      pressureGrid->set(i, j, 0.0);
    }
  }
  return pressureGrid;
}

glm::vec2 Simulator::maxVelocity(VelocityGrid const *const velocity){
  glm::vec2 maxVec = velocity->getCell(0,0);
  for(unsigned int i = 0; i < w; i++){
    for(unsigned int j = 0; j < h; j++){
      if(glm::length(maxVec) < glm::length(velocity->getCell(i,j))){
        maxVec = velocity->getCell(i,j);
      }
    }
  }
  return maxVec;
}

float Simulator::calculateDeltaT(glm::vec2 maxV, glm::vec2 gravity){
  //TODO: glm::length(gravity) should be changed to something else relating to gravity
  float max = glm::length(maxV) + sqrt(abs(5*gridSize*glm::length(gravity)));
  float dT = 5*gridSize/max;
  return dT;
}

float Simulator::getDeltaT(){
  return deltaT;
}
