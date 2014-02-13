#pragma once
#include <glm/glm.hpp>
template<typename T>
class OrdinalGrid;
template<typename T>
class Grid;

class Simulator;
class VelocityGrid;

class State {
public:
  State(unsigned int width, unsigned int height);
  ~State();
  Grid<bool>const *const getBoundaryGrid() const;
  VelocityGrid const *const getVelocityGrid() const;
  OrdinalGrid<glm::vec3> const *const getInkGrid() const;
  
  void setFluidGrid(Grid<bool> const* const);
  void setBoundaryGrid(Grid<bool> const* const);
  void setVelocityGrid(VelocityGrid const* const);
  void setInkGrid(OrdinalGrid<glm::vec3> const* const);
  
  unsigned int getW();
  unsigned int getH();

private:
  void resetVelocityGrids();

  //  OrdinalGrid<double> *pressureGrid;
  OrdinalGrid<glm::vec3> *inkGrid;
  Grid<bool> *fluidGrid;
  Grid<bool> *boundaryGrid;
  VelocityGrid *velocityGrid;
  unsigned int w, h;

  friend class Simulator;
};
