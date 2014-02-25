#pragma once
#include <glm/glm.hpp>
#include <vector>
template<typename T>
class OrdinalGrid;
template<typename T>
class Grid;

class Simulator;
struct VelocityGrid;

enum BoundaryType{
  SOLID = -1, EMPTY = 0, FLUID = 1
};

class State {
public:
  State(unsigned int width, unsigned int height);
  ~State();
  Grid<BoundaryType>const *const getBoundaryGrid() const;
  VelocityGrid const *const getVelocityGrid() const;
  OrdinalGrid<glm::vec3> const *const getInkGrid() const;
  OrdinalGrid<float> const *const getSignedDistanceGrid() const;
  

  void setBoundaryGrid(Grid<BoundaryType> const* const);
  void setVelocityGrid(VelocityGrid const* const);
  void setInkGrid(OrdinalGrid<glm::vec3> const* const);
  void setSignedDistanceGrid(OrdinalGrid<float> const* const);
  
  unsigned int getW();
  unsigned int getH();

private:
  void resetVelocityGrids();
  
  OrdinalGrid<float> *signedDistanceGrid;
  OrdinalGrid<glm::vec3> *inkGrid;
  Grid<BoundaryType> *boundaryGrid;
  VelocityGrid *velocityGrid;
  unsigned int w, h;
  //  LevelSet *levelSet;


  friend class Simulator;
};
