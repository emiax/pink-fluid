#pragma once
template<typename T>
class OrdinalGrid;
class State;
struct PressureSolver{
	virtual ~PressureSolver(){}
	virtual void solve(OrdinalGrid<float> const* const divergenceGrid, State const* const state, OrdinalGrid<double> *pressureGrid, const float dt) = 0;
};