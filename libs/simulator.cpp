#include <simulator.h>
#include <state.h>
#include <ordinalGrid.h>
#include <velocityGrid.h>
#include <algorithm>
#include <iostream>
#include <glm/ext.hpp>
#include <cassert>
#include <levelSet.h>


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

void Simulator::updateMarkers(float dt) {
  resetCellTypeGrid(stateTo);
  Grid<CellType> *from = stateFrom->levelSet->cellTypeGrid;
  Grid<CellType> *to = stateTo->levelSet->cellTypeGrid;
  to->setForEach([&](unsigned int i, unsigned int j){
      if (from->get(i, j) == CellType::SOLID) {
        return CellType::SOLID;
      } 
      else{
        return CellType::FLUID;
      }
    });
}


/**
 * Simulator step function. Iterates the simulation one time step, dt.
 * @param dt Time step, deltaT
 */
void Simulator::step(float dt) {
  
  updateMarkers(dt);
  advect(stateFrom, stateTo, dt);
  
  // applyGravity(stateTo, glm::vec2(0,0.01), dt);

  calculateDivergence(stateTo, divergenceGrid);
  jacobiIteration(stateTo, 100, dt);
  gradientSubtraction(stateTo, dt);

  // copycellTypeGrid(stateFrom, stateTo);  
  stateTo->levelSet->reinitialize();

  deltaT = calculateDeltaT(maxVelocity(stateTo->velocityGrid), glm::vec2(0,0.01));

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

  // signed distance
  writeTo->levelSet->distanceGrid->setForEach([&](unsigned int i, unsigned int j){
    glm::vec2 position = backTrackMid(readFrom, i, j, dt);
    return readFrom->levelSet->distanceGrid->getCrerp(position);
  });
}

/**
 * Copy cellTypeGrid
 * @param readFrom State to read from
 * @param writeTo State to write to
 */
void Simulator::resetCellTypeGrid(State* writeTo) {
  writeTo->levelSet->cellTypeGrid->setForEach([&](unsigned int i, unsigned int j){
    return CellType::EMPTY;
  });
}

// void Simulator::copycellTypeGrid(State const *readFrom, State *writeTo){
//   for(unsigned int i = 0; i < w; i++){
//     for(unsigned int j = 0; j < h; j++){
//       writeTo->cellTypeGrid->set(i,j, readFrom->cellTypeGrid->get(i,j));
//     }
//   }
// }

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
  Grid<CellType> const *const cellTypeGrid = state->getCellTypeGrid();

  velocityGrid->u->setForEach([&](unsigned int i, unsigned int j){
      if (cellTypeGrid->get(i, j) == CellType::FLUID) {
        return velocityGrid->u->get(i,j)+g.x*deltaT;
      }
      return velocityGrid->u->get(i,j);
    });
  velocityGrid->v->setForEach([&](unsigned int i, unsigned int j){
      if (cellTypeGrid->get(i, j) == CellType::FLUID) {
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
  Grid<CellType> const *const cellTypeGrid = readFrom->getCellTypeGrid();

  for(unsigned int i = 0; i < w; i++){
    for(unsigned int j = 0; j < h; j++){

      if (cellTypeGrid->get(i, j) == CellType::FLUID) {
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
  Grid<CellType> const *const cellTypeGrid = readFrom->getCellTypeGrid();
  
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
        if (cellTypeGrid->get(i, j) != CellType::FLUID) {
          continue;
        }

        double pL = 0;
        if (cellTypeGrid->get(i - 1, j) != CellType::SOLID) {
          pL = pressureGridFrom->get(i - 1, j);
          neighbouringFluidCells++;
        } 
        double pR = 0;
        if (cellTypeGrid->get(i + 1, j) != CellType::SOLID) {
          pR = pressureGridFrom->get(i + 1, j);
          neighbouringFluidCells++;
        } 
        double pU = 0;
        if (cellTypeGrid->get(i, j - 1) != CellType::SOLID) {
          pU = pressureGridFrom->get(i, j - 1);
          neighbouringFluidCells++;
        } 
        double pD = 0;
        if (cellTypeGrid->get(i, j + 1) != CellType::SOLID) {
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
  Grid<CellType> const *const cellTypeGrid = state->getCellTypeGrid();

  // looping through pressure cells
  for (unsigned int j = 0; j < h; ++j) {
    for (unsigned int i = 0; i < w; ++i) {
      // is current cell solid?
      if (cellTypeGrid->get(i, j) == CellType::FLUID) {
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
      if (cellTypeGrid->get(i, j) == CellType::SOLID) {

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
