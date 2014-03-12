#include <micSolver.h>
#include <ordinalGrid.h>
#include <state.h>
MICSolver::MICSolver(int size){
	solver = PCGSolver<double>();
	aMatrix = new SparseMatrix<double>(size, 5);
	xVector = new std::vector<double>(size);
	bVector = new std::vector<double>(size);
}
bool MICSolver::solve(OrdinalGrid<float> const* const divergenceGrid, State const* const state, OrdinalGrid<double> *pressureGrid, const float dt){
	fillA(aMatrix, state, dt);
	fillB(bVector, divergenceGrid);
	double residual;
	int iterations;
	if(solver.solve(*aMatrix, *bVector, *xVector, residual, iterations)){
		std::cout << "Pressure Solver status: Success" << std::endl;
		return true;
	}
	else{
		std::cout << "Pressure Solver status: Failed with " << iterations << " iterations and "
		<<  residual << " as a residual" <<  std::endl;
		return false;
	}
	pressureGrid->setForEach([&](unsigned int i, unsigned int j){
		return xVector->at(pressureGrid->indexTranslation(i,j));
	});
}
void MICSolver::fillA(SparseMatrix<double> *aMatrix, State const* const state, const float dt){
	aMatrix->zero();
	int w = state->getCellTypeGrid()->getW();
	int h = state->getCellTypeGrid()->getH();
	for(auto j = 0u; j < h; j++){
		for(auto i = 0u; i < w; i++){
			CellType ct0 = state->getCellTypeGrid()->get(i,j);
			unsigned int diag = state->getCellTypeGrid()->indexTranslation(i,j);
			double scale = dt;
			if(ct0 == CellType::FLUID){
				auto indexTranslation = [&](unsigned int i, unsigned int j){
					return state->getCellTypeGrid()->indexTranslation(i, j);
				};
				if(state->getCellTypeGrid()->isValid(i+1,j)){
					CellType ct = state->getCellTypeGrid()->get(i+1,j);
					if(ct == CellType::FLUID){
						aMatrix->add_to_element(indexTranslation(i,j), indexTranslation(i,j), scale);
						aMatrix->add_to_element(indexTranslation(i+1,j), indexTranslation(i+1,j), scale);
						aMatrix->set_element(indexTranslation(i+1,j), indexTranslation(i,j), -scale);
						aMatrix->set_element(indexTranslation(i,j), indexTranslation(i+1,j), -scale);
					}
					else if(ct == CellType::EMPTY){
						aMatrix->add_to_element(indexTranslation(i,j), indexTranslation(i,j), scale);
					}
				}

				if(state->getCellTypeGrid()->isValid(i,j+1)){
					CellType ct = state->getCellTypeGrid()->get(i,j+1);
					if(ct == CellType::FLUID){
						aMatrix->add_to_element(indexTranslation(i,j), indexTranslation(i,j), scale);
						aMatrix->add_to_element(indexTranslation(i,j+1), indexTranslation(i,j+1), scale);
						aMatrix->set_element(indexTranslation(i,j+1), indexTranslation(i,j), -scale);
						aMatrix->set_element(indexTranslation(i,j), indexTranslation(i,j+1), -scale);
					}
					else if(ct == CellType::EMPTY){
						aMatrix->add_to_element(indexTranslation(i,j), indexTranslation(i,j), scale);
					}
				}
			}
		}
	}
	// for (int j = 0; j < w*h; ++j){
	// 	for (int i = 0; i < w*h; ++i){
	// 		std::cout << (*aMatrix)(i,j) << " ";
	// 	}
	// 	std::cout << std::endl;
	// }
	// std::cin.get();
}
void MICSolver::fillB(std::vector<double> *bVector, OrdinalGrid<float> const* const divergenceGrid){
	for(auto i = 0u; i < divergenceGrid->size(); i++){
		bVector->at(i) = divergenceGrid->get(i);
	}
}