#include <simulator.h>
#include <state.h>
#include <ordinalGrid.h>
#include <velocityGrid.h>
#include <algorithm>
#include <iostream>
#include <glm/ext.hpp>
#include <cassert>
#include <levelSet.h>
#include <jacobiIteration.h>
#include <micSolver.h>
#include <particle.h>
#include <particleTracker.h>

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
  // pressureSolver = new JacobiIteration(100);
  pressureSolver = new MICSolver(w*h);

  pTracker = new ParticleTracker(w, h, PARTICLES_PER_CELL);
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

  glm::vec2 gravity = glm::vec2(0, 1);

  stateFrom->levelSet->reinitialize();
  pTracker->reinitializeParticles(stateFrom->getSignedDistanceGrid());
  extrapolateVelocity(stateFrom, stateFrom);

  advect(stateFrom, stateTo, dt);
  pTracker->advect(stateFrom->velocityGrid, dt);

  stateTo->levelSet->updateCellTypes();
  applyGravity(stateTo, gravity, dt);

  calculateDivergence(stateTo, divergenceGrid);

  pressureSolver->solve(divergenceGrid, stateTo, pressureGridTo, dt);

  gradientSubtraction(stateTo, dt);

  calculateDivergence(stateTo, divergenceGrid);

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
  #pragma omp parallel sections
  {
    //X
    #pragma omp section
    writeTo->velocityGrid->u->setForEach([&](unsigned int i, unsigned int j){
      glm::vec2 position = util::advect::mac::backTrackU(readFrom->velocityGrid, i, j, dt);
      return readFrom->velocityGrid->u->getCrerp(position);
    });

    //Y
    #pragma omp section
    writeTo->velocityGrid->v->setForEach([&](unsigned int i, unsigned int j){
      glm::vec2 position = util::advect::mac::backTrackV(readFrom->velocityGrid, i, j, dt);
      return readFrom->velocityGrid->v->getCrerp(position);
    });

    // level set distance grid
    #pragma omp section
    writeTo->levelSet->distanceGrid->setForEach([&](unsigned int i, unsigned int j){
      glm::vec2 position = util::advect::backTrack(readFrom->velocityGrid, i, j, dt);
      return readFrom->levelSet->distanceGrid->getCrerp(position);
    });
  }
}

/**
 * Apply gravity to fluid cells
 * @param state  state with velocity grid to be augmented
 * @param g      gravity vector
 * @param deltaT time step, dt
 */
void Simulator::applyGravity(State *state, glm::vec2 g, float deltaT){
  VelocityGrid *velocityGrid = state->velocityGrid;
  Grid<CellType> const *const cellTypeGrid = state->getCellTypeGrid();

  velocityGrid->u->setForEach([&](unsigned int i, unsigned int j){
      if (cellTypeGrid->get(i, j) == CellType::FLUID || cellTypeGrid->clampGet(i - 1, j) == CellType::FLUID) {
        return velocityGrid->u->get(i,j)+g.x*deltaT;
      }
      return velocityGrid->u->get(i,j);
    });

  velocityGrid->v->setForEach([&](unsigned int i, unsigned int j){
      if (cellTypeGrid->get(i, j) == CellType::FLUID || cellTypeGrid->clampGet(i, j - 1) == CellType::FLUID) {
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
  const float scale = 1.0f;
  OrdinalGrid<float> *u = readFrom->velocityGrid->u;
  OrdinalGrid<float> *v = readFrom->velocityGrid->v;
  Grid<CellType> const *const cellTypeGrid = readFrom->getCellTypeGrid();
  float volumeError = readFrom->levelSet->getVolumeError();

  toDivergenceGrid->setForEach([&](unsigned int i, unsigned int j){
    if (cellTypeGrid->get(i, j) == CellType::FLUID) {
        float divergence = -scale * (u->get(i+1,j) - u->get(i,j) + v->get(i,j+1) - v->get(i,j));
        
        if(cellTypeGrid->isValid(i-1,j) && cellTypeGrid->get(i-1,j) == CellType::SOLID) {
          divergence -= scale*u->get(i,j);
        }
        if(cellTypeGrid->isValid(i+1,j) && cellTypeGrid->get(i+1,j) == CellType::SOLID) {
          divergence += scale*u->get(i+1,j);
        }

        if(cellTypeGrid->isValid(i,j-1) && cellTypeGrid->get(i,j-1) == CellType::SOLID) {
          divergence -= scale*v->get(i,j);
        }
        if(cellTypeGrid->isValid(i,j+1) && cellTypeGrid->get(i,j+1) == CellType::SOLID) {
          divergence += scale*v->get(i,j+1);
        }

        return divergence + volumeError; // non-scientific adjustment for volume loss
      } else {
        return 0.0f;
      }
    });
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
      // std::cout << std::setw(5) << pressureGridTo->get(i, j) << " ";
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
   // std::cout << std::endl;
  }

  // enforce boundary condition
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

void Simulator::initializeExtrapolation(State *stateFrom) {

  const Grid<CellType> *cellTypeGrid = stateFrom->getCellTypeGrid();
  VelocityGrid *velocityGrid = stateFrom->velocityGrid;

  // u velocities
  for (unsigned j = 0; j < h; ++j) {
    for (unsigned i = 0; i <= w; ++i) {
      
      GridCoordinate leftCell = GridCoordinate(i - 1, j);
      GridCoordinate rightCell = GridCoordinate(i, j);

      if (cellTypeGrid->clampGet(leftCell) == CellType::EMPTY && 
        cellTypeGrid->clampGet(rightCell) == CellType::EMPTY) {

        GridCoordinate farLeft = GridCoordinate(i - 2, j);
        GridCoordinate upLeft = GridCoordinate(i - 1, j - 1);
        GridCoordinate downLeft = GridCoordinate(i - 1, j + 1);

        GridCoordinate farRight = GridCoordinate(i + 1, j);
        GridCoordinate upRight = GridCoordinate(i, j - 1);
        GridCoordinate downRight = GridCoordinate(i, j + 1);

        float u = 0;
        unsigned int nContributions = 0;

        // left cells
        if (cellTypeGrid->clampGet(farLeft) == CellType::FLUID) {
          u += velocityGrid->u->get(i - 1, j);
          ++nContributions;
        }

        // right cells
        if (cellTypeGrid->clampGet(farRight) == CellType::FLUID) {
          u += velocityGrid->u->get(i + 1, j);
          ++nContributions;
        }
        
        // up cells
        if (cellTypeGrid->clampGet(upLeft) == CellType::FLUID ||
            cellTypeGrid->clampGet(upRight) == CellType::FLUID) {
          u += velocityGrid->u->get(i, j - 1);
          ++nContributions;
        }

        // down cells
        if (cellTypeGrid->clampGet(downLeft) == CellType::FLUID ||
            cellTypeGrid->clampGet(downRight) == CellType::FLUID) {
          u += velocityGrid->u->get(i, j + 1);
          ++nContributions;
        }

        velocityGrid->u->set(i, j, u/nContributions);
      }
    }
  }

  // v velocities
  for (unsigned j = 0; j <= h; ++j) {
    for (unsigned i = 0; i < w; ++i) {

      GridCoordinate upCell = GridCoordinate(i, j - 1);
      GridCoordinate downCell = GridCoordinate(i, j);

      if (cellTypeGrid->clampGet(upCell) == CellType::EMPTY && 
        cellTypeGrid->clampGet(downCell) == CellType::EMPTY) {

        GridCoordinate farUp = GridCoordinate(i, j - 2);
        GridCoordinate leftUp = GridCoordinate(i - 1, j - 1);
        GridCoordinate rightUp = GridCoordinate(i + 1, j - 1);

        GridCoordinate farDown = GridCoordinate(i, j + 1);
        GridCoordinate leftDown = GridCoordinate(i - 1 , j);
        GridCoordinate rightDown = GridCoordinate(i + 1, j);

        float v = 0;
        unsigned int nContributions = 0;

        // up cells
        if (cellTypeGrid->clampGet(farUp) == CellType::FLUID) {
          v += velocityGrid->v->get(i, j - 1);
          ++nContributions;
        }

        // down cells
        if (cellTypeGrid->clampGet(farDown) == CellType::FLUID) {
          v += velocityGrid->v->get(i, j + 1);
          ++nContributions;
        }
        
        // left cells
        if (cellTypeGrid->clampGet(leftUp) == CellType::FLUID ||
            cellTypeGrid->clampGet(leftDown) == CellType::FLUID) {
          v += velocityGrid->v->get(i - 1, j);
          ++nContributions;
        }

        // right cells
        if (cellTypeGrid->clampGet(rightUp) == CellType::FLUID ||
            cellTypeGrid->clampGet(rightDown) == CellType::FLUID) {
          v += velocityGrid->v->get(i + 1, j);
          ++nContributions;
        }

        velocityGrid->v->set(i, j, v/(float)nContributions);
      }


    }
  }

}

void Simulator::extrapolateVelocity(State *stateFrom, State *stateTo) {

  initializeExtrapolation(stateFrom);
  const Grid<glm::vec2> *closestPointGrid = stateFrom->levelSet->getClosestPointGrid();

  VelocityGrid *fromVelocityGrid = stateFrom->velocityGrid;
  VelocityGrid *toVelocityGrid = stateTo->velocityGrid;
  const Grid<CellType> *cellTypeGrid = stateFrom->getCellTypeGrid();
  
  unsigned int w = stateFrom->w;
  unsigned int h = stateFrom->h;

  for(unsigned j = 0; j < h; ++j) {
    for(unsigned i = 0; i <= w; ++i) {

      glm::vec2 currentPoint = glm::vec2(i - 0.5, j);
      GridCoordinate leftCell = GridCoordinate(i - 1, j);
      GridCoordinate rightCell = GridCoordinate(i, j);

      if (cellTypeGrid->clampGet(leftCell) != CellType::EMPTY || 
        cellTypeGrid->clampGet(rightCell) != CellType::EMPTY) {
        continue;
      }

      glm::vec2 leftClosestPoint = closestPointGrid->clampGet(leftCell);
      glm::vec2 rightClosestPoint = closestPointGrid->clampGet(rightCell);

      float dLeft = glm::distance(currentPoint, leftClosestPoint);
      float dRight = glm::distance(currentPoint, rightClosestPoint);

      float uLeft = fromVelocityGrid->u->getLerp(leftClosestPoint.x + 0.5, leftClosestPoint.y);
      float uRight = fromVelocityGrid->u->getLerp(rightClosestPoint.x + 0.5, rightClosestPoint.y);

      float t = dLeft/(dLeft + dRight);
      toVelocityGrid->u->set(i, j, t*uRight + (1.0f - t)*uLeft);
    }
  }


  for(unsigned j = 0; j <= h; ++j) {
    for(unsigned i = 0; i < w; ++i) {

      glm::vec2 currentPoint = glm::vec2(i, j - 0.5);
      GridCoordinate upCell = GridCoordinate(i, j - 1);
      GridCoordinate downCell = GridCoordinate(i, j);

      if (cellTypeGrid->clampGet(upCell) != CellType::EMPTY || 
          cellTypeGrid->clampGet(downCell) != CellType::EMPTY) {
        continue;
      }

      glm::vec2 upClosestPoint = closestPointGrid->clampGet(upCell);
      glm::vec2 downClosestPoint = closestPointGrid->clampGet(downCell);

      float dUp = glm::distance(currentPoint, upClosestPoint);
      float dDown = glm::distance(currentPoint, downClosestPoint);

      float vUp = fromVelocityGrid->v->getLerp(upClosestPoint.x, upClosestPoint.y + 0.5);
      float vDown = fromVelocityGrid->v->getLerp(downClosestPoint.x, downClosestPoint.y + 0.5);

      float t = dUp/(dUp + dDown);
      toVelocityGrid->v->set(i, j, t*vDown + (1.0f - t)*vUp);
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
  float dT = glm::min(5*gridSize/max, 1.0f);
  return dT;
}

float Simulator::getDeltaT(){
  return deltaT;
}
