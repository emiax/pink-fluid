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
  LevelSet(unsigned int w, unsigned int h, unsigned int d, SignedDistanceFunction sdf, Grid<CellType> const* const boundaries);
  LevelSet(const LevelSet& origin);
  LevelSet(unsigned int w, unsigned int h, unsigned int d, SignedDistFunc sdf = [](int,int,int){return 1.0;}, std::function<CellType (unsigned int i, unsigned int j, unsigned int k)> = [](int,int,int){return CellType::FLUID;});

  ~LevelSet();
  // LevelSet(unsigned int w, unsigned int h, Grid<CellType> const* const ctg);

  Grid<CellType> const *const getCellTypeGrid() const;
  Grid<bool> const *const getDoneGrid() const;
  OrdinalGrid<float> const *const getDistanceGrid() const;
  Grid<glm::vec3> const *const getClosestPointGrid() const;

  void setCellTypeGrid(Grid<CellType> const* const);

  void reinitialize();
  void updateCellTypes();
  float getVolumeError();

  OrdinalGrid<float> *distanceGrid;
  Grid<CellType> *cellTypeGrid;
  SignedDistanceFunction *initSDF;

  std::ostream& write(std::ostream&);
  std::istream& read(std::istream&);

 private:
  void updateInterfaceNeighbors();
  void updateInterfaceNeighborCell(unsigned int i, unsigned int j, unsigned int k);
  void updateNeighborsFrom(GridCoordinate from);

  void updateFromCell(GridCoordinate to, GridCoordinate from);
  void updateFromCell(unsigned int xTo, unsigned int yTo, unsigned int xFrom, unsigned int yFrom);

  void updateCurrentVolume();

  void fastMarch();

  static bool heapCompare(GridCoordinate &a, GridCoordinate &b);

  static int sgn(const float &val);

  void initializeDistanceGrid(SignedDistanceFunction sdf);
  void clampInfiniteCells();

  static constexpr float INF = 9999999.0f;

  Grid<bool> *doneGrid;
  Grid<glm::vec3> *closestPointGrid;
  GridHeap *gridHeap;
  unsigned int heapEnd;
  OrdinalGrid<float> *oldDistanceGrid;

  int w, h, d;
  float targetVolume, currentVolume;

};
