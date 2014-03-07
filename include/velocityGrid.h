#pragma once
#include <glm/glm.hpp>
#include <ordinalGrid.h>
#include <interfaces/quantity.h>
#include <util.h>
struct VelocityGrid{
  VelocityGrid(unsigned int w, unsigned int h, unsigned int d);
  ~VelocityGrid();

  glm::vec2 getCell(unsigned int i, unsigned int j, unsigned int k) const;
  glm::vec2 getLerp(glm::vec2 p) const;
  OrdinalGrid<float> *u, *v, *w; 
};
