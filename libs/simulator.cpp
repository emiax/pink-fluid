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
#include <gpuAdvector.h>

Simulator::Simulator(State *sf, State *st, float scale) : stateFrom(sf), stateTo(st), gridSize(scale){

  // grid dims must be equal
  assert(sf->getW() == st->getW());
  assert(sf->getH() == st->getH());
  assert(sf->getD() == st->getD());

  w = sf->getW();
  h = sf->getH();
  d = sf->getD();

  // init non-state grids

  gpuAdvector = new GpuAdvector(w, h, d);
  divergenceGrid = new OrdinalGrid<float>(w, h, d);
  pressureGridFrom = new OrdinalGrid<double>(w, h, d);
  pressureGridTo = new OrdinalGrid<double>(w, h, d);
  pressureSolver = new MICSolver(w*h*d);
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

  glm::vec3 gravity = glm::vec3(0, -1, 0);

  stateFrom->levelSet->reinitialize();
  extrapolateVelocity(stateFrom, stateFrom);

  gpuAdvector->advect(stateFrom, stateTo, dt);
  //  advect(stateFrom, stateTo, dt);
  applyGravity(stateTo, gravity, dt);
  //  stateTo->levelSet->updateCellTypes();

  calculateNegativeDivergence(stateTo, divergenceGrid);
  pressureSolver->solve(divergenceGrid, stateTo, pressureGridTo, dt);
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

  
#pragma omp parallel sections
  {
    // X
#pragma omp section
    writeTo->velocityGrid->u->setForEach([&](unsigned int i, unsigned int j, unsigned int k){
        glm::vec3 position = util::advect::mac::backTrackU(readFrom->velocityGrid, i, j, k, dt);
        return readFrom->velocityGrid->u->getCrerp(position);
      });

    // Y
#pragma omp section
    writeTo->velocityGrid->v->setForEach([&](unsigned int i, unsigned int j, unsigned int k){
        glm::vec3 position = util::advect::mac::backTrackV(readFrom->velocityGrid, i, j, k, dt);
        return readFrom->velocityGrid->v->getCrerp(position);
      });

    // Z
#pragma omp section
    writeTo->velocityGrid->w->setForEach([&](unsigned int i, unsigned int j, unsigned int k) {
        glm::vec3 position = util::advect::mac::backTrackW(readFrom->velocityGrid, i, j, k, dt);
        return readFrom->velocityGrid->w->getCrerp(position);
      });


    //Z
#pragma omp section
    writeTo->velocityGrid->w->setForEach([&](unsigned int i, unsigned int j, unsigned int k){
        glm::vec3 position = util::advect::mac::backTrackW(readFrom->velocityGrid, i, j, k, dt);
        return readFrom->velocityGrid->w->getCrerp(position);
      });

    // level set distance grid
#pragma omp section
    writeTo->levelSet->distanceGrid->setForEach([&](unsigned int i, unsigned int j, unsigned int k){
        glm::vec3 position = util::advect::backTrack(readFrom->velocityGrid, i, j, k, dt);
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
    void Simulator::applyGravity(State *state, glm::vec3 g, float deltaT){
      VelocityGrid *velocityGrid = state->velocityGrid;
      Grid<CellType> const *const cellTypeGrid = state->getCellTypeGrid();

      // u velocities
      velocityGrid->u->setForEach([&](unsigned int i, unsigned int j, unsigned int k){
          if (i < w && cellTypeGrid->get(i, j, k) != CellType::SOLID) {
            return velocityGrid->u->get(i, j, k)+g.x*deltaT;
          }
          return velocityGrid->u->get(i, j, k);
        });

      // v velocities
      velocityGrid->v->setForEach([&](unsigned int i, unsigned int j, unsigned int k){
          if (j < h && cellTypeGrid->get(i, j, k) != CellType::SOLID) {
            return velocityGrid->v->get(i, j, k)+g.y*deltaT;
          }
          return velocityGrid->v->get(i, j, k);
        });

      // w velocities
      velocityGrid->w->setForEach([&](unsigned int i, unsigned int j, unsigned int k){
          if (k < d && cellTypeGrid->get(i, j, k) != CellType::SOLID) {
            return velocityGrid->w->get(i, j, k)+g.z*deltaT;
          }
          return velocityGrid->w->get(i, j, k);
        });
    }


    /**
     * Calculate divergence.
     * @param readFrom State to read from
     * @param toDivergenceGrid An ordinal grid of floats to write the divergences to
     */
    void Simulator::calculateNegativeDivergence(State const* readFrom, OrdinalGrid<float>* toDivergenceGrid) {
      const float scale = 1.0f;
      OrdinalGrid<float> *u = readFrom->velocityGrid->u;
      OrdinalGrid<float> *v = readFrom->velocityGrid->v;
      OrdinalGrid<float> *w = readFrom->velocityGrid->w;
      Grid<CellType> const *const cellTypeGrid = readFrom->getCellTypeGrid();

      float volumeError = readFrom->levelSet->getVolumeError();

      toDivergenceGrid->setForEach([&](unsigned int i, unsigned int j, unsigned int k){
          if (cellTypeGrid->get(i, j, k) == CellType::FLUID) {
            float entering = u->get(i, j, k) + v->get(i, j, k) + w->get(i, j, k);
            float leaving = u->get(i + 1, j, k) + v->get(i, j + 1, k) + w->get(i, j, k + 1);

            float divergence = leaving - entering;

            return - divergence + volumeError; // non-scientific adjustment for volume loss
          } else {
            return 0.0f;
          }
        });
    }


/*toDivergenceGrid->setForEach([&](unsigned int i, unsigned int j){

  if (cellTypeGrid->get(i, j) == CellType::FLUID) {
  float divergence = -scale * (u->get(i+1,j,k) - u->get(i,j,k) + v->get(i,j+1,k) - v->get(i,j,k));

  if(cellTypeGrid->isValid(i-1,j, k) && cellTypeGrid->get(i-1,j, k) == CellType::SOLID) {
  divergence -= scale*u->get(i,j,k);
  }
  if(cellTypeGrid->isValid(i+1,j) && cellTypeGrid->get(i+1,j) == CellType::SOLID) {
  divergence += scale*u->get(i+1,j,k);
  }

  if(cellTypeGrid->isValid(i,j-1,k) && cellTypeGrid->get(i,j-1,k) == CellType::SOLID) {
  divergence -= scale*v->get(i,j,k);
  }
  if(cellTypeGrid->isValid(i,j+1,k) && cellTypeGrid->get(i,j+1,k) == CellType::SOLID) {
  divergence += scale*v->get(i,j+1,k);
  }


  if(cellTypeGrid->isValid(i,j, k-1) && cellTypeGrid->get(i,j, k-1) == CellType::SOLID) {
  divergence -= scale*v->get(i,j, k);
  }
  if(cellTypeGrid->isValid(i,j, k+1) && cellTypeGrid->get(i,j, k+1) == CellType::SOLID) {
  divergence += scale*v->get(i,j, k+1);
  }

  return divergence;
  } else {
  return 0.0f;
  }
  });
  }*/

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
  OrdinalGrid<float> *wVelocityGrid = state->velocityGrid->w;
  Grid<CellType> const *const cellTypeGrid = state->getCellTypeGrid();

  // looping through pressure cells
#pragma omp parallel for default(none) shared(uVelocityGrid, vVelocityGrid, wVelocityGrid)
      for(unsigned int k = 0; k < d; ++k) {
        for (unsigned int j = 0; j < h; ++j) {
          for (unsigned int i = 0; i < w; ++i) {
            // is current cell solid?
            if (cellTypeGrid->get(i, j, k) == CellType::FLUID) {
              float uL = uVelocityGrid->get(i, j, k);
              float uR = uVelocityGrid->get(i+1, j, k);

              float vU = vVelocityGrid->get(i, j, k);
              float vD = vVelocityGrid->get(i, j+1, k);

              float wF = wVelocityGrid->get(i, j, k);
              float wB = wVelocityGrid->get(i, j, k + 1);

              float p = pressureGridTo->get(i, j, k);

              uL -= scale * p;
              uR += scale * p;
              vU -= scale * p;
              vD += scale * p;
              wF -= scale * p;
              wB += scale * p;

              uVelocityGrid->set(i, j, k, uL);
              uVelocityGrid->set(i + 1, j, k, uR);
              vVelocityGrid->set(i, j, k, vU);
              vVelocityGrid->set(i, j + 1, k, vD);
              wVelocityGrid->set(i, j, k, wF);
              wVelocityGrid->set(i, j, k + 1, wB);
            }
          }
        }

#pragma omp parallel for default(none) shared(uVelocityGrid, vVelocityGrid, wVelocityGrid)
        for(unsigned k = 0; k < d; ++k) {
          for (unsigned int j = 0; j < h; ++j) {
            for (unsigned int i = 0; i < w; ++i) {
              // is current cell solid?
              if (cellTypeGrid->get(i, j, k) == CellType::SOLID) {

                uVelocityGrid->set(i, j, k, 0.0f);
                uVelocityGrid->set(i + 1, j, k, 0.0f);
                vVelocityGrid->set(i, j, k, 0.0f);
                vVelocityGrid->set(i, j + 1, k, 0.0f);
                wVelocityGrid->set(i, j, k, 0.0f);
                wVelocityGrid->set(i, j, k + 1, 0.0f);
              }
            }
          }
        }
      }
    }


    /**
     * Method to extrapolate velocity values from the water region to the air/empty region.
     * @param stateFrom
     * @param stateTo
     */
    void Simulator::extrapolateVelocity(State *stateFrom, State *stateTo) {
      const Grid<glm::vec3> *closestPointGrid = stateFrom->levelSet->getClosestPointGrid();

      VelocityGrid *fromVelocityGrid = stateFrom->velocityGrid;
      VelocityGrid *toVelocityGrid = stateTo->velocityGrid;
      const Grid<CellType> *cellTypeGrid = stateFrom->getCellTypeGrid();

      unsigned int w = stateFrom->w;
      unsigned int h = stateFrom->h;
      unsigned int d = stateFrom->d;


      // u velocities
      toVelocityGrid->u->setForEach([&](int i, int j, int k) {
          glm::vec3 currentPoint = glm::vec3(i - 0.5, j, k);
          GridCoordinate leftCell = GridCoordinate(i - 1, j, k);
          GridCoordinate rightCell = GridCoordinate(i, j, k);

          if (cellTypeGrid->clampGet(leftCell) != CellType::EMPTY ||
              cellTypeGrid->clampGet(rightCell) != CellType::EMPTY) {
            return fromVelocityGrid->u->get(i, j, k);

          }

          glm::vec3 leftClosestPoint = closestPointGrid->clampGet(leftCell);
          glm::vec3 rightClosestPoint = closestPointGrid->clampGet(rightCell);


          float dLeft = glm::distance(currentPoint, leftClosestPoint);
          float dRight = glm::distance(currentPoint, rightClosestPoint);

          float uLeft = fromVelocityGrid->u->getNearest(leftClosestPoint.x + 0.5, leftClosestPoint.y, leftClosestPoint.z);
          float uRight = fromVelocityGrid->u->getNearest(rightClosestPoint.x + 0.5, rightClosestPoint.y, rightClosestPoint.z);

          float t = dLeft/(dLeft + dRight);
          return t*uRight + (1.0f - t)*uLeft;

        });


      // v velocities
      toVelocityGrid->v->setForEach([&](int i, int j, int k) {
          glm::vec3 currentPoint = glm::vec3(i, j - 0.5, k);
          GridCoordinate upCell = GridCoordinate(i, j - 1, k);
          GridCoordinate downCell = GridCoordinate(i, j, k);

          if (cellTypeGrid->clampGet(upCell) != CellType::EMPTY ||
              cellTypeGrid->clampGet(downCell) != CellType::EMPTY) {
            return fromVelocityGrid->v->get(i, j, k);
          }

          glm::vec3 upClosestPoint = closestPointGrid->clampGet(upCell);
          glm::vec3 downClosestPoint = closestPointGrid->clampGet(downCell);

          float dUp = glm::distance(currentPoint, upClosestPoint);
          float dDown = glm::distance(currentPoint, downClosestPoint);

          float vUp = fromVelocityGrid->v->getNearest(upClosestPoint.x, upClosestPoint.y + 0.5, upClosestPoint.z);
          float vDown = fromVelocityGrid->v->getNearest(downClosestPoint.x, downClosestPoint.y + 0.5, downClosestPoint.z);

          float t = dUp/(dUp + dDown);
          return t*vDown + (1.0f - t)*vUp;
        });

      // w velocities
      toVelocityGrid->w->setForEach([&](int i, int j, int k) {
          glm::vec3 currentPoint = glm::vec3(i, j, k - 0.5);
          GridCoordinate frontCell = GridCoordinate(i, j, k - 1);
          GridCoordinate backCell = GridCoordinate(i, j, k);

          if (cellTypeGrid->clampGet(frontCell) != CellType::EMPTY ||
              cellTypeGrid->clampGet(backCell) != CellType::EMPTY) {
            return fromVelocityGrid->w->get(i, j, k);
          }

          glm::vec3 frontClosestPoint = closestPointGrid->clampGet(frontCell);
          glm::vec3 backClosestPoint = closestPointGrid->clampGet(backCell);

          float dFront = glm::distance(currentPoint, frontClosestPoint);
          float dBack = glm::distance(currentPoint, backClosestPoint);

          float wFront = fromVelocityGrid->v->getNearest(frontClosestPoint.x, frontClosestPoint.y, frontClosestPoint.z + 0.5);
          float wBack = fromVelocityGrid->v->getNearest(backClosestPoint.x, backClosestPoint.y + 0.5, backClosestPoint.z + 0.5);

          float t = dFront/(dFront + dBack);
          return t*wBack + (1.0f - t)*wFront;
        });
    }


    /**
     * Reset pressure grid
     */
    OrdinalGrid<double>* Simulator::resetPressureGrid() {
      for (unsigned k = 0; k < d; ++k) {
        for (unsigned j = 0; j < h; ++j) {
          for (unsigned i = 0; i < w; ++i) {
            pressureGridFrom->set(i, j, k, 0.0);
            pressureGridTo->set(i, j, k, 0.0);
          }
        }
      }
      return pressureGridFrom;
    }


    /**
     * Find the maximum velocity present in the grid
     * @param  velocity velocity grid to sample from
     * @return maxVec   max velocity vector
     */
    glm::vec3 Simulator::maxVelocity(VelocityGrid const *const velocity){

      glm::vec3 maxVec = velocity->getCell(0, 0, 0);

      //  std::cout << maxVec.x << ", " << maxVec.y << " " << maxVec.z << std::endl;

      for (unsigned k = 0; k < d; ++k) {
        for (unsigned j = 0; j < h; j++) {
          for (unsigned i = 0; i < w; i++) {
            if (glm::length(maxVec) < glm::length(velocity->getCell(i, j, k))) {
              maxVec = velocity->getCell(i, j, k);
            }
          }
        }
      }
      return maxVec;
    }


    OrdinalGrid<float>* Simulator::getDivergenceGrid() {
      return divergenceGrid;
    }

    /**
     * Calculate max deltaT for the current iteration.
     * This calculation is based on the max velocity in the grid.
     * @param  maxV     max velocity in the velocity grid
     * @param  gravity  gravitational force vector
     * @return dT       maximum time step length
     */
    float Simulator::calculateDeltaT(glm::vec3 maxV, glm::vec3 gravity){
      //TODO: glm::length(gravity) should be changed to something else relating to gravity
      float max = glm::length(maxV) + sqrt(abs(5*gridSize*glm::length(gravity)));
      float dT = glm::min(5*gridSize/max, 1.0f);
      return dT;
    }

    float Simulator::getDeltaT(){
      return deltaT;
    }
