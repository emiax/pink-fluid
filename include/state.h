#pragma once
#include <glm/glm.hpp>
#include <vector>
template<typename T>
class OrdinalGrid;
template<typename T>
class Grid;

class LevelSet;
class Simulator;
struct VelocityGrid;

enum CellType{
  SOLID = -1, EMPTY = 0, FLUID = 1
};

class State {
public:
  State(unsigned int width, unsigned int height);
  ~State();
  Grid<CellType>const *const getCellTypeGrid() const;
  VelocityGrid const *const getVelocityGrid() const;
  OrdinalGrid<glm::vec3> const *const getInkGrid() const;
  OrdinalGrid<float> const *const getSignedDistanceGrid() const;
  Grid<glm::vec2> const *const getClosestPointGrid() const;
  
  void setCellTypeGrid(Grid<CellType> const* const);
  void setVelocityGrid(VelocityGrid const* const);
  void setInkGrid(OrdinalGrid<glm::vec3> const* const);
  void setLevelSet(LevelSet *ls);
  
  unsigned int getW() const;
  unsigned int getH() const;

private:
  void resetVelocityGrids();
  
  OrdinalGrid<glm::vec3> *inkGrid;
  VelocityGrid *velocityGrid;
  unsigned int w, h;
  LevelSet *levelSet;

  friend class Simulator;
};
