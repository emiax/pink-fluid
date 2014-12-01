#pragma once
#include <glm/glm.hpp>
#include <ordinalGrid.h>
#include <util.h>
struct VelocityGrid{
  VelocityGrid(unsigned int w, unsigned int h, unsigned int d);
  ~VelocityGrid();
  
  glm::vec3 getCell(unsigned int i, unsigned int j, unsigned int k) const;
  glm::vec3 getLerp(glm::vec3 p) const;
  OrdinalGrid<float> *u, *v, *w;

  std::ostream& write(std::ostream&);
};
