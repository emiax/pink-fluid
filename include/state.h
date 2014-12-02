#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <particleTracker.h>
#include <bubbleTracker.h>

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
  State(unsigned int width, unsigned int height, unsigned int depth);
  ~State();
  Grid<CellType>const *const getCellTypeGrid() const;
  VelocityGrid const *const getVelocityGrid() const;
  OrdinalGrid<glm::vec3> const *const getInkGrid() const;
  OrdinalGrid<float> const *const getSignedDistanceGrid() const;
  Grid<glm::vec3> const *const getClosestPointGrid() const;
  
  void setCellTypeGrid(Grid<CellType> const* const);
  void setVelocityGrid(VelocityGrid const* const);
  void setInkGrid(OrdinalGrid<glm::vec3> const* const);
  void setLevelSet(LevelSet *ls);
  
  unsigned int getW() const;
  unsigned int getH() const;
  unsigned int getD() const;

  std::ostream& write(std::ostream &stream);
  std::istream& read(std::istream &stream);

  void setBubbleTracker(BubbleTracker*);
  void setParticleTracker(ParticleTracker*);

private:
  void resetVelocityGrids();
  
  OrdinalGrid<glm::vec3> *inkGrid;
  VelocityGrid *velocityGrid;
  unsigned int w, h, d;
  LevelSet *levelSet;

  BubbleTracker *bubbleTracker;
  ParticleTracker *particleTracker;

  friend class Simulator;
};
