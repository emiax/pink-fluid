#pragma once
#include <interfaces/pressureSolver.h>
class JacobiIteration : public PressureSolver{
public:
	JacobiIteration(int maxIterations = 100);
	virtual bool solve(OrdinalGrid<float> const* const divergenceGrid, State const* const state, OrdinalGrid<double> *pressureGrid, const float dt);
private:
	int maxIterations;
};