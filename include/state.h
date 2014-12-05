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
  State(const State& origin);
  ~State();
  Grid<CellType>const *const getCellTypeGrid() const;
  VelocityGrid const *const getVelocityGrid() const;
  std::vector<Bubble> getBubbles() const;

  OrdinalGrid<float> const *const getSignedDistanceGrid() const;
  Grid<glm::vec3> const *const getClosestPointGrid() const;
  
  void setCellTypeGrid(Grid<CellType> const* const);
  void setVelocityGrid(VelocityGrid const* const);
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
  
  VelocityGrid *velocityGrid;
  unsigned int w, h, d;
  LevelSet *levelSet;

  BubbleTracker *bubbleTracker = nullptr;
  ParticleTracker *particleTracker = nullptr;

  std::vector<Bubble> bubbleState;
  
  friend class Simulator;
};
