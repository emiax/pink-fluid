#pragma once
#include <glm/glm.hpp>
#include <ordinalGrid.h>

struct VelocityGrid{
  VelocityGrid(unsigned int w, unsigned int h);
  ~VelocityGrid();
  glm::vec2 getCell(unsigned int i, unsigned int j) const;
  OrdinalGrid<float> *u, *v; 
};