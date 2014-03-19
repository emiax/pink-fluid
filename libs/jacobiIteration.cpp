#include <jacobiIteration.h>
#include <ordinalGrid.h>
#include <state.h>
#include <algorithm>
JacobiIteration::JacobiIteration(int maxIterations){
  this->maxIterations = maxIterations;
}

bool JacobiIteration::solve(OrdinalGrid<float> const* const divergenceGrid, State const* const state, OrdinalGrid<double> *pressureGridTo, const float dt){
  const float sqDeltaX = 1.0f;
  Grid<CellType> const *const cellTypeGrid = state->getCellTypeGrid();
  const unsigned int w = state->getW();
  const unsigned int h = state->getH();
  const unsigned int d = state->getD();
  OrdinalGrid<double> *pressureGridFrom = new OrdinalGrid<double>(w,h,d);
  auto pTo = pressureGridTo;
  auto pFrom = pressureGridFrom;

  for(unsigned int iteration = 0; iteration < maxIterations; ++iteration) {
#pragma omp parallel for collapse(2)
    for (unsigned int k = 0; k < d; ++k) {
      for (unsigned int j = 0; j < h; ++j) {
        for (unsigned int i = 0; i < w; ++i) {
          
          float divergence;
          int neighbouringFluidCells = 0;

          // is current cell solid?
          if (cellTypeGrid->get(i, j, k) != CellType::FLUID) {
            pressureGridTo->set(i, j, k, 0);
            continue;
          }

          double pL = 0;
          if(cellTypeGrid->isValid(i-1,j,k)){
            if (cellTypeGrid->get(i - 1, j, k) != CellType::SOLID) {
              pL = pressureGridFrom->get(i - 1, j, k);
              neighbouringFluidCells++;
            }
          }

          double pR = 0;
          if(cellTypeGrid->isValid(i + 1, j, k)){
            if (cellTypeGrid->get(i + 1, j, k) != CellType::SOLID) {
              pR = pressureGridFrom->get(i + 1, j, k);
              neighbouringFluidCells++;
            }
          }

          double pU = 0;
          if(cellTypeGrid->isValid(i,j-1, k)){
            if (cellTypeGrid->get(i, j - 1, k) != CellType::SOLID) {
              pU = pressureGridFrom->get(i, j - 1, k);
              neighbouringFluidCells++;
            }

          }

          double pD = 0;
          if(cellTypeGrid->isValid(i,j+1, k)){
            if (cellTypeGrid->get(i, j + 1, k) != CellType::SOLID) {
              pD = pressureGridFrom->get(i, j + 1, k);
              neighbouringFluidCells++;
            }
          }

          double pF = 0;
          if(cellTypeGrid->isValid(i,j, k-1)){
            if (cellTypeGrid->get(i, j, k - 1) != CellType::SOLID) {
              pU = pressureGridFrom->get(i, j, k - 1);
              neighbouringFluidCells++;
            }
          }

          double pB = 0;
          if(cellTypeGrid->isValid(i,j, k+1)){
            if (cellTypeGrid->get(i, j, k+1) != CellType::SOLID) {
              pD = pressureGridFrom->get(i, j, k+1);
              neighbouringFluidCells++;
            }
          }

          divergence = divergenceGrid->get(i, j, k);
          
          // discretized poisson equation
          double p = pL + pR + pU + pD + pF + pB + sqDeltaX * divergence/dt;
          p /= ((double) neighbouringFluidCells);
          pressureGridTo->set(i, j, k, p);
          
        }
      }
    }
    std::swap(pressureGridFrom, pressureGridTo);
  }
  delete pressureGridFrom;
  return true;
}
