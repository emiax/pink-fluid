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

/**
 * Simulator step function. Iterates the simulation one time step, dt.
 * @param dt Time step, deltaT
 */
void Simulator::step(float dt) {

  glm::vec2 gravity = glm::vec2(0, 0.10);

  // simulation stack
  advect(stateFrom, stateTo, dt);
  applyGravity(stateTo, gravity, dt);

  stateTo->levelSet->reinitialize();
  extrapolateVelocity(stateTo, stateTo);

  calculateDivergence(stateTo, divergenceGrid);
  jacobiIteration(stateTo, 100, dt);
  gradientSubtraction(stateTo, dt);

  deltaT = calculateDeltaT(maxVelocity(stateTo->velocityGrid), gravity);
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

  // level set distance grid
  writeTo->levelSet->distanceGrid->setForEach([&](unsigned int i, unsigned int j){
      glm::vec2 position = backTrackMid(readFrom, i, j, dt);
      return readFrom->levelSet->distanceGrid->getCrerp(position);
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
  Grid<CellType> const *const cellTypeGrid = state->getCellTypeGrid();

  velocityGrid->u->setForEach([&](unsigned int i, unsigned int j){
      if (cellTypeGrid->get(i, j) != CellType::SOLID) {
        return velocityGrid->u->get(i,j)+g.x*deltaT;
      }
      return velocityGrid->u->get(i,j);
    });

  velocityGrid->v->setForEach([&](unsigned int i, unsigned int j){
      if (cellTypeGrid->get(i, j) != CellType::SOLID) {
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


void Simulator::extrapolateVelocity(State *stateFrom, State *stateTo) {
  const Grid<glm::vec2> *closestPointGrid = stateFrom->levelSet->getClosestPointGrid();

  VelocityGrid *fromVelocityGrid = stateFrom->velocityGrid;
  VelocityGrid *toVelocityGrid = stateTo->velocityGrid;
  const Grid<CellType> *cellTypeGrid = stateFrom->getCellTypeGrid();
  
  unsigned int w = stateFrom->w;
  unsigned int h = stateFrom->h;

  toVelocityGrid->u->setForEach([&](int i, int j) {
    glm::vec2 currentPoint = glm::vec2(i - 0.5, j);
    GridCoordinate leftCell = GridCoordinate(i - 1, j);
    GridCoordinate rightCell = GridCoordinate(i, j);

    if (cellTypeGrid->clampGet(leftCell) == CellType::FLUID || 
        cellTypeGrid->clampGet(rightCell) == CellType::FLUID) {
      return fromVelocityGrid->u->get(i, j);
    }

    glm::vec2 leftClosestPoint = closestPointGrid->clampGet(leftCell);
    glm::vec2 rightClosestPoint = closestPointGrid->clampGet(rightCell);

    float dLeft = glm::distance(currentPoint, leftClosestPoint);
    float dRight = glm::distance(currentPoint, rightClosestPoint);

    float uLeft = fromVelocityGrid->u->getLerp(leftClosestPoint.x + 0.5, leftClosestPoint.y);
    float uRight = fromVelocityGrid->u->getLerp(rightClosestPoint.x + 0.5, rightClosestPoint.y);

    float t = dLeft/(dLeft + dRight);
    return t*uRight + (1.0f - t)*uLeft;
    
  });


  toVelocityGrid->v->setForEach([&](int i, int j) {
    glm::vec2 currentPoint = glm::vec2(i, j - 0.5);
    GridCoordinate upCell = GridCoordinate(i, j - 1);
    GridCoordinate downCell = GridCoordinate(i, j);

    if (cellTypeGrid->clampGet(upCell) == CellType::FLUID || 
        cellTypeGrid->clampGet(downCell) == CellType::FLUID) {
      return fromVelocityGrid->v->get(i, j);
    }

    glm::vec2 upClosestPoint = closestPointGrid->clampGet(upCell);
    glm::vec2 downClosestPoint = closestPointGrid->clampGet(downCell);

    float dUp = glm::distance(currentPoint, upClosestPoint);
    float dDown = glm::distance(currentPoint, downClosestPoint);

    float vUp = fromVelocityGrid->v->getLerp(upClosestPoint.x, upClosestPoint.y + 0.5);
    float vDown = fromVelocityGrid->v->getLerp(downClosestPoint.x, downClosestPoint.y + 0.5);

    float t = dUp/(dUp + dDown);
    return t*vDown + (1.0f - t)*vUp;
  });
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
    float dT = glm::min(5*gridSize/max, 1.0f);
    return dT;
  }

  float Simulator::getDeltaT(){
    return deltaT;
  }
