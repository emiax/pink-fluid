#include <simulator.h>
#include <state.h>
#include <ordinalGrid.h>
#include <velocityGrid.h>
#include <algorithm>
#include <iostream>
#include <glm/ext.hpp>
#include <cassert>

Simulator::Simulator(State *sf, State *st, float scale) : stateFrom(sf), stateTo(st), gridSize(scale){

  // grid dims must be equal
  assert( sf->getW() == st->getW() );
  assert( sf->getH() == st->getH() );
  
  w = sf->getW();
  h = sf->getH();

  // init non-state grids
  divergenceGrid = new OrdinalGrid<float>(w, h);
  pressureGridFrom = new OrdinalGrid<double>(w, h);
  pressureGridTo = new OrdinalGrid<double>(w, h);
}

/**
 * Destructor.
 */
Simulator::~Simulator() {
  delete divergenceGrid;
  delete pressureGridFrom;
  delete pressureGridTo;
}


/**
 * 
 */
void Simulator::addAdvectedMarker(glm::vec2 p, float dt) {
  glm::vec2 q = p + stateFrom->velocityGrid->getLerp(p)*dt;
  unsigned int i = std::round(glm::clamp(q.x, 0.0f, (float)(w-1)));
  unsigned int j = std::round(glm::clamp(q.y, 0.0f, (float)(h-1)));
  stateTo->boundaryGrid->set(i, j, BoundaryType::FLUID);
}


void Simulator::updateMarkers(float dt) {
  resetBoundaryGrid(stateTo);
  /*for (int i = 0; i < w; i++) {
    for(int j = 0; j < h; j++) {
      if (stateFrom->boundaryGrid->get(i, j) == BoundaryType::FLUID) {
                addAdvectedMarker(glm::vec2(i - 0.25f, j - 0.25f), dt);
                addAdvectedMarker(glm::vec2(i - 0.25f, j + 0.25f), dt);
                addAdvectedMarker(glm::vec2(i + 0.25f, j - 0.25f), dt);
                addAdvectedMarker(glm::vec2(i + 0.25f, j + 0.25f), dt);
      }
    }
    }*/
  Grid<BoundaryType> *from = stateFrom->boundaryGrid;
  Grid<BoundaryType> *to = stateTo->boundaryGrid;
  to->setForEach([&](unsigned int i, unsigned int j){
      if (from->get(i, j) == BoundaryType::SOLID) {
        return BoundaryType::SOLID;
      } 
      else{
        return BoundaryType::FLUID;
      }
    });
}


/**
 * Simulator step function. Iterates the simulation one time step, dt.
 * @param dt Time step, deltaT
 */
void Simulator::step(float dt) {
  
  // OrdinalGrid<float> *divergenceOut = new OrdinalGrid<float>(w, h);

  //  copyBoundaries(stateFrom, stateTo);
  updateMarkers(dt);
  advect(stateFrom, stateTo, dt);
  
  // applyGravity(stateTo, glm::vec2(0,0.01), dt);

  calculateDivergence(stateTo, divergenceGrid);
  jacobiIteration(stateTo, 100, dt);
  gradientSubtraction(stateTo, dt);

  // calculateDivergence(stateTo, divergenceOut);
  
  // // divergence sum
  // float sumDivIn = 0;
  // float sumDivOut = 0;
  //   for (unsigned int j = 1; j < h-1; ++j) {
  //     for (unsigned int i = 1; i < w-1; ++i) {
  //       sumDivIn += fabs(divergenceGrid->get(i, j));
  //       sumDivOut += fabs(divergenceOut->get(i, j));
  //     }
  //   }
    //  std::cout << "avg. div in: " << sumDivIn / (w*h) << std::endl;
    //  std::cout << "avg. div out: " << sumDivOut / (w*h) << std::endl;
    //  std::cout << std::endl;
  //  std::cin.get();

  // Variable time step calculation
  deltaT = calculateDeltaT(maxVelocity(stateTo->velocityGrid), glm::vec2(0,0.1));

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
  writeTo->velocityGrid->u->setForEach([&](unsigned int i, unsigned int j){
    glm::vec2 position = backTrackU(readFrom, i, j, dt);
    return readFrom->velocityGrid->u->getCrerp(position);
  });

  //Y
  writeTo->velocityGrid->v->setForEach([&](unsigned int i, unsigned int j){
    glm::vec2 position = backTrackV(readFrom, i, j, dt);
    return readFrom->velocityGrid->v->getCrerp(position);
  });

  // ink grid
  // writeTo->inkGrid->setForEach([&](unsigned int i, unsigned int j){
  //   glm::vec2 position = backTrackMid(readFrom, i, j, dt);
  //   return readFrom->inkGrid->getCrerp(position);
  // });

  // signed distance
  writeTo->signedDistanceGrid->setForEach([&](unsigned int i, unsigned int j){
    glm::vec2 position = backTrackMid(readFrom, i, j, dt);
    return readFrom->signedDistanceGrid->getCrerp(position);
  });  

}

/**
 * Copy boundaries
 * @param readFrom State to read from
 * @param writeTo State to write to
 */
void Simulator::resetBoundaryGrid(State* writeTo) {
  writeTo->boundaryGrid->setForEach([&](unsigned int i, unsigned int j){
      return BoundaryType::EMPTY;
    });
}


/**
 * Find the previous position of the temporary particle in the grid that travelled to i, j.
 * @param readFrom State to read from
 * @param i   x coordinate
 * @param j   y coordinate
 * @param dt  the time between the two steps
 * @return    position to copy new value from
 */
glm::vec2 Simulator::backTrackU(State const* readFrom, int i, int j, float dt){
  glm::vec2 position(i,j);
  glm::vec2 v = glm::vec2(readFrom->velocityGrid->u->get(i,j),
                          readFrom->velocityGrid->v->getLerp(i-0.5,j+0.5));
  glm::vec2 midPos = position - (dt/2) * v;

  glm::vec2 midV = glm::vec2(readFrom->velocityGrid->u->getLerp(midPos),
                             readFrom->velocityGrid->v->getLerp(midPos.x-0.5, midPos.y+0.5));
  return position-dt*midV;
}

glm::vec2 Simulator::backTrackV(State const* readFrom, int i, int j, float dt){
  glm::vec2 position(i,j);
  glm::vec2 v = glm::vec2(readFrom->velocityGrid->u->get(i+0.5,j-0.5),
                          readFrom->velocityGrid->v->getLerp(i,j));
  glm::vec2 midPos = position - (dt/2) * v;

  glm::vec2 midV = glm::vec2(readFrom->velocityGrid->u->getLerp(midPos.x+0.5, midPos.y-0.5),
                             readFrom->velocityGrid->v->getLerp(midPos));
  return position-dt*midV;
}

glm::vec2 Simulator::backTrackMid(State const* readFrom, int i, int j, float dt){
  glm::vec2 position(i,j);
  glm::vec2 v = glm::vec2(readFrom->velocityGrid->u->getLerp(i+0.5,j),
                          readFrom->velocityGrid->v->getLerp(i,j+0.5));
  glm::vec2 midPos = position - (dt/2) * v;

  glm::vec2 midV = glm::vec2(readFrom->velocityGrid->u->getLerp(midPos.x+0.5f, midPos.y),
                             readFrom->velocityGrid->v->getLerp(midPos.x, midPos.y+0.5f));
  return position-dt*midV;
}


//Apply gravity to fluid cells
void Simulator::applyGravity(State *state, glm::vec2 g, float deltaT){
  VelocityGrid *velocityGrid = state->velocityGrid;
  Grid<BoundaryType> *boundaryGrid = state->boundaryGrid;

  velocityGrid->u->setForEach([&](unsigned int i, unsigned int j){
      if (boundaryGrid->get(i, j) == BoundaryType::FLUID) {
        return velocityGrid->u->get(i,j)+g.x*deltaT;
      }
      return velocityGrid->u->get(i,j);
    });
  velocityGrid->v->setForEach([&](unsigned int i, unsigned int j){
      if (boundaryGrid->get(i, j) == BoundaryType::FLUID) {
        return velocityGrid->v->get(i,j)+g.y*deltaT;
      }
      return velocityGrid->v->get(i,j);
    });
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

      if (readFrom->boundaryGrid->get(i, j) == BoundaryType::FLUID) {
        float entering = u->get(i, j) + v->get(i, j);
        float leaving = u->get(i + 1, j) + v->get(i, j + 1);

        float divergence = leaving - entering;


        toDivergenceGrid->set(i, j, divergence);
      } else {
        toDivergenceGrid->set(i, j, 0);

      }
    }
  }
}


/**
 * Perform nIterantions Jacobi iterations in order to calculate pressures.
 * @param readFrom The state to read from
 * @param nIterations number of iterations
 */
void Simulator::jacobiIteration(State const* readFrom, unsigned int nIterations, float dt) {

  const float sqDeltaX = 1.0f;
  Grid<BoundaryType> const* const boundaryGrid = readFrom->getBoundaryGrid();
  
  resetPressureGrid();

  for(unsigned int k = 0; k < nIterations; ++k) {

    OrdinalGrid<double> *tmp = pressureGridFrom;
    pressureGridFrom = pressureGridTo;
    pressureGridTo = tmp;

    for (unsigned int j = 0; j < h; ++j) {
      for (unsigned int i = 0; i < w; ++i) {

        float divergence;
        int neighbouringFluidCells = 0;

        // is current cell solid?
        if (boundaryGrid->get(i, j) != BoundaryType::FLUID) {
          continue;
        }

        double pL = 0;
        if (boundaryGrid->get(i - 1, j) != BoundaryType::SOLID) {
          pL = pressureGridFrom->get(i - 1, j);
          neighbouringFluidCells++;
        } 
        double pR = 0;
        if (boundaryGrid->get(i + 1, j) != BoundaryType::SOLID) {
          pR = pressureGridFrom->get(i + 1, j);
          neighbouringFluidCells++;
        } 
        double pU = 0;
        if (boundaryGrid->get(i, j - 1) != BoundaryType::SOLID) {
          pU = pressureGridFrom->get(i, j - 1);
          neighbouringFluidCells++;
        } 
        double pD = 0;
        if (boundaryGrid->get(i, j + 1) != BoundaryType::SOLID) {
          pD = pressureGridFrom->get(i, j + 1);
          neighbouringFluidCells++;
        }

        divergence = divergenceGrid->get(i, j);

        // discretized poisson equation
        double p = pL + pR + pU + pD - sqDeltaX * divergence/dt;
        p /= ((double) neighbouringFluidCells);

        pressureGridTo->set(i, j, p);
      }
    }

  }
}

/**
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
  Grid<BoundaryType> const* const boundaries = state->getBoundaryGrid();

  // looping through pressure cells
  for (unsigned int j = 0; j < h; ++j) {
    for (unsigned int i = 0; i < w; ++i) {
      // is current cell solid?
      if (boundaries->get(i, j) == BoundaryType::FLUID) {
        float uL = uVelocityGrid->get(i, j);
        float uR = uVelocityGrid->get(i+1, j);

        float vU = vVelocityGrid->get(i, j);
        float vD = vVelocityGrid->get(i, j+1);

        float p = pressureGridTo->get(i, j);

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

  for (unsigned int j = 0; j < h; ++j) {
    for (unsigned int i = 0; i < w; ++i) {
      // is current cell solid?
      if (boundaries->get(i, j) == BoundaryType::SOLID) {

        uVelocityGrid->set(i, j, 0.0f);
        uVelocityGrid->set(i+1, j, 0.0f);
        vVelocityGrid->set(i, j, 0.0f);
        vVelocityGrid->set(i, j+1, 0.0f);
      } 
    }
  }

}

/**
 * Reset pressure grid
 */
OrdinalGrid<double>* Simulator::resetPressureGrid() {
  for (unsigned int j = 0; j < h; ++j) {
        for (unsigned int i = 0; i < w; ++i) {
      pressureGridFrom->set(i, j, 0.0);
      pressureGridTo->set(i, j, 0.0);
    }
  }
  return pressureGridFrom;
}

OrdinalGrid<float>* Simulator::getDivergenceGrid() {
  return divergenceGrid;
}

/**
 * Find the maximum velocity present in the grid
 * @param  velocity velocity grid to sample from
 * @return maxVec   max velocity vector
 */
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

/**
 * Calculate max deltaT for the current iteration.
 * This calculation is based on the max velocity in the grid.
 * @param  maxV     max velocity in the velocity grid
 * @param  gravity  gravitational force vector
 * @return dT       maximum time step length
 */
float Simulator::calculateDeltaT(glm::vec2 maxV, glm::vec2 gravity){
  //TODO: glm::length(gravity) should be changed to something else relating to gravity
  float max = glm::length(maxV) + sqrt(abs(5*gridSize*glm::length(gravity)));
  float dT = 5*gridSize/max;
  return dT;
}

float Simulator::getDeltaT(){
  return deltaT;
}
