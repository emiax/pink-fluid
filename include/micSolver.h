#pragma once
#include <interfaces/pressureSolver.h>
#include <pcgsolver/pcg_solver.h>
#include <vector>

template<class T>
struct SparseMatrix;



class MICSolver : public PressureSolver{
	public:
		MICSolver(int size);
		virtual void solve(OrdinalGrid<float> const* const divergenceGrid, State const* const state, OrdinalGrid<double> *pressureGrid, const float dt);
		void fillA(SparseMatrix<double> *aMatrix, State const* const state, const float dt);
		void fillB(std::vector<double> *bVector, OrdinalGrid<float> const* const divergenceGrid);
	private:
		PCGSolver<double> solver;
		SparseMatrix<double> *aMatrix;
		std::vector<double> *xVector;
		std::vector<double> *bVector;

};