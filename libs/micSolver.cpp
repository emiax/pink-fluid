#include <micSolver.h>
#include <ordinalGrid.h>
#include <state.h>

MICSolver::MICSolver(int size){
  solver = PCGSolver<double>();
  solver.set_solver_parameters(1e-6, 100, 0.97, 0.25);
  aMatrix = new SparseMatrix<double>(size, 5);
  xVector = new std::vector<double>(size);
  bVector = new std::vector<double>(size);
}
bool MICSolver::solve(OrdinalGrid<float> const* const divergenceGrid, State const* const state, OrdinalGrid<double> *pressureGrid, const float dt){
  fillA(aMatrix, state, dt);
  fillB(bVector, divergenceGrid);
  double residual;
  int iterations;
  bool solved = true;
  if(solver.solve(*aMatrix, *bVector, *xVector, residual, iterations)){
    // std::cout << "Pressure Solver status: Success" << std::endl;
    // std::cout << "iterations = " << iterations << std::endl;
  }
  else{
    std::cerr << "Pressure Solver status: Failed with " << iterations << " iterations and "
              <<  residual << " as a residual" <<  std::endl;
    solved = false;
  }
  pressureGrid->setForEach([&](unsigned int i, unsigned int j, unsigned int k){
      return xVector->at(pressureGrid->indexTranslation(i,j,k));
    });
  return solved;
}
void MICSolver::fillA(SparseMatrix<double> *aMatrix, State const* const state, const float dt){
  aMatrix->zero();
  int w = state->getCellTypeGrid()->getW();
  int h = state->getCellTypeGrid()->getH();
  int d = state->getCellTypeGrid()->getD();
  unsigned int row = 0;

  for(auto k = 0u; k < d; k++){
    for(auto j = 0u; j < h; j++){
      for(auto i = 0u; i < w; i++){

        CellType ct0 = state->getCellTypeGrid()->get(i,j,k);
        double scale = dt;

        if(ct0 == CellType::FLUID){
          auto indexTranslation = [&](unsigned int i, unsigned int j, unsigned int k){
            return state->getCellTypeGrid()->indexTranslation(i, j, k);
          };

          // left
          if(state->getCellTypeGrid()->isValid(i-1,j,k)){
            CellType ct = state->getCellTypeGrid()->get(i-1,j,k);
            if(ct == CellType::FLUID){
              aMatrix->add_to_element(row, row, scale);
              aMatrix->set_element(row, indexTranslation(i-1,j,k), -scale);
            }
            else if(ct == CellType::EMPTY){
              aMatrix->add_to_element(row, row, scale);
            }
          } else {
            aMatrix->add_to_element(row, row, scale);
          }

          // right
          if(state->getCellTypeGrid()->isValid(i+1,j,k)){
            CellType ct = state->getCellTypeGrid()->get(i+1,j,k);
            if(ct == CellType::FLUID){
              aMatrix->add_to_element(row, row, scale);
              aMatrix->set_element(row, indexTranslation(i+1,j,k), -scale);
            }
            else if(ct == CellType::EMPTY){
              aMatrix->add_to_element(row, row, scale);
            }
          } else {
            aMatrix->add_to_element(row, row, scale);
          }

          // up
          if(state->getCellTypeGrid()->isValid(i,j-1,k)){
            CellType ct = state->getCellTypeGrid()->get(i,j-1,k);
            if(ct == CellType::FLUID){
              aMatrix->add_to_element(row, row, scale);
              aMatrix->set_element(row, indexTranslation(i,j-1,k), -scale);
            }
            else if(ct == CellType::EMPTY){
              aMatrix->add_to_element(row, row, scale);
            }
          } else {
            aMatrix->add_to_element(row, row, scale);
          }

          // down
          if(state->getCellTypeGrid()->isValid(i,j+1,k)){
            CellType ct = state->getCellTypeGrid()->get(i,j+1,k);
            if(ct == CellType::FLUID){
              aMatrix->add_to_element(row, row, scale);
              aMatrix->set_element(row, indexTranslation(i,j+1,k), -scale);
            }
            else if(ct == CellType::EMPTY){
              aMatrix->add_to_element(row, row, scale);
            }
          } else {
            aMatrix->add_to_element(row, row, scale);
          }

          // front
          if(state->getCellTypeGrid()->isValid(i,j,k-1)){
            CellType ct = state->getCellTypeGrid()->get(i,j,k-1);
            if(ct == CellType::FLUID){
              aMatrix->add_to_element(row, row, scale);
              aMatrix->set_element(row, indexTranslation(i,j,k-1), -scale);
            }
            else if(ct == CellType::EMPTY){
              aMatrix->add_to_element(row, row, scale);
            }
          } else {
            aMatrix->add_to_element(row, row, scale);
          }

          // back
          if(state->getCellTypeGrid()->isValid(i,j,k+1)){
            CellType ct = state->getCellTypeGrid()->get(i,j,k+1);
            if(ct == CellType::FLUID){
              aMatrix->add_to_element(row, row, scale);
              aMatrix->set_element(row, indexTranslation(i,j,k+1), -scale);
            }
            else if(ct == CellType::EMPTY){
              aMatrix->add_to_element(row, row, scale);
            }
          } else {
            aMatrix->add_to_element(row, row, scale);
          }

        }
        row++;
      }
    }
    // for (int j = 0; j < w*h; ++j){
    //  for (int i = 0; i < w*h; ++i){
    //          std::cout << (*aMatrix)(i,j);
    //  }
    //  std::cout << std::endl;
    // }
  }
}

void MICSolver::fillB(std::vector<double> *bVector, OrdinalGrid<float> const* const divergenceGrid){
  for(auto i = 0u; i < divergenceGrid->size(); i++){
    bVector->at(i) = divergenceGrid->get(i);
  }
}
