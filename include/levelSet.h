#pragma once

#include <glm/glm.hpp>
#include <state.h>
#include <ordinalGrid.h>
#include <grid.h>
class VelocityGrid;
class LevelSet{
public:
  LevelSet(unsigned int w, unsigned int h);
  void initializeLevelSet(Grid<CellType> const *const);
  OrdinalGrid<CellType> const *const advectLevelSet(VelocityGrid const* const);
  OrdinalGrid<CellType> const *const getsetCellTypeGrid();
  Grid<bool> const *const getDoneGrid() const;
private:
  OrdinalGrid<glm::vec2> *distanceGrid;
  Grid<bool> *doneGrid;
  int w,h;
};