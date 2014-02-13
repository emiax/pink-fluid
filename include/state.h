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
  VelocityGrid const *const getVelocityGrid() const;
  void setVelocityGrid(VelocityGrid*);
  unsigned int getW();
  unsigned int getH();
private:
  void resetVelocityGrids();
  VelocityGrid *velocityGrid;
  unsigned int w, h;

  friend class Simulator;
};
