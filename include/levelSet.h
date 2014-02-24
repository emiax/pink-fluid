#pragma once

#include <glm/glm.hpp>
#include <state.h>
#include <ordinalGrid.h>
#include <grid.h>
class VelocityGrid;
class LevelSet{
public:
  LevelSet(unsigned int w, unsigned int h);
  void initializeLevelSet(Grid<BoundaryType> const *const);
  OrdinalGrid<BoundaryType> const *const advectLevelSet(VelocityGrid const* const);
  OrdinalGrid<BoundaryType> const *const getBoundaryGrid();
  Grid<bool> const *const getDoneGrid();
private:
  OrdinalGrid<glm::vec2> *distanceGrid;
  Grid<bool> *doneGrid;
  int w,h;
};
