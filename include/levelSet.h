#pragma once

#include <glm/glm.hpp>
#include <state.h>
#include <ordinalGrid.h>
#include <grid.h>
#include <signedDistanceFunction.h>
class VelocityGrid;

class LevelSet{
public:
  LevelSet(unsigned int w, unsigned int h, SignedDistanceFunction sdf);

  
  OrdinalGrid<CellType> const *const getCellTypeGrid();
  Grid<bool> const *const getDoneGrid() const;
  OrdinalGrid<float> const *const getDistanceGrid() const;

  void reinitialize(LevelSet const *LevelSetFrom);

  OrdinalGrid<float> *distanceGrid;
  Grid<CellType> *cellTypeGrid;

private:
  void markClosestAsDone(LevelSet const *LevelSetFrom);
  void fastSweep(LevelSet const *LevelSetFrom);

  void initializeDistanceGrid(SignedDistanceFunction sdf);
  Grid<bool> *doneGrid;
  int w,h;
};
