#pragma once

#include <glm/glm.hpp>
#include <state.h>
#include <ordinalGrid.h>
#include <grid.h>
#include <signedDistanceFunction.h>
#include <limits>
class VelocityGrid;
class GridHeap;

class LevelSet{
public:
  LevelSet(unsigned int w, unsigned int h, SignedDistanceFunction sdf, Grid<CellType> const* const boundaries);
  // LevelSet(unsigned int w, unsigned int h, Grid<CellType> const* const ctg);

  Grid<CellType> const *const getCellTypeGrid() const;
  Grid<bool> const *const getDoneGrid() const;
  OrdinalGrid<float> const *const getDistanceGrid() const;
  Grid<glm::vec2> const *const getClosestPointGrid() const;

  void setCellTypeGrid(Grid<CellType> const* const);

  void reinitialize();

  OrdinalGrid<float> *distanceGrid;
  Grid<CellType> *cellTypeGrid;
  SignedDistanceFunction *initSDF;

private:
  void updateInterfaceNeighbors();
  void updateInterfaceNeighborCell(unsigned int i, unsigned int j);
  void updateNeighborsFrom(unsigned int i, unsigned int j);

  void updateFromCell(unsigned int xTo, unsigned int yTo, unsigned int xFrom, unsigned int yFrom);
  void updateCellTypes();

  void fastMarch();

  static bool heapCompare(GridCoordinate &a, GridCoordinate &b);

  static int sgn(float &val);

  void initializeDistanceGrid(SignedDistanceFunction sdf);

  static constexpr float INF = 9999999.0f;
  
  Grid<bool> *doneGrid;
  Grid<glm::vec2> *closestPointGrid;
  GridHeap *gridHeap;
  unsigned int heapEnd;
  int w, h;
};
