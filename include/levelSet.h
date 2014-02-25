#pragma once

#include <glm/glm.hpp>
#include <state.h>
#include <ordinalGrid.h>
#include <grid.h>
#include <signedDistanceFunction.h>
class VelocityGrid;

class LevelSet{
public:
  LevelSet(unsigned int w, unsigned int h, SignedDistanceFunction sdf, Grid<CellType> const* const boundaries);
  // LevelSet(unsigned int w, unsigned int h, Grid<CellType> const* const ctg);

  Grid<CellType> const *const getCellTypeGrid() const;
  Grid<bool> const *const getDoneGrid() const;
  OrdinalGrid<float> const *const getDistanceGrid() const;

  void setCellTypeGrid(Grid<CellType> const* const);

  void reinitialize();

  OrdinalGrid<float> *distanceGrid;
  Grid<CellType> *cellTypeGrid;
  SignedDistanceFunction *initSDF;

private:
  void markClosestAsDone();
  void fastSweep();
  void updateCellTypes();

  void initializeDistanceGrid(SignedDistanceFunction sdf);
  Grid<bool> *doneGrid;
  int w,h;
};
