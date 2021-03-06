#pragma once
#ifndef UTIL_H
#define UTIL_H
#include <glm/glm.hpp>

struct VelocityGrid;

namespace util {
	namespace advect{
    /**
     * Find the previous position of the temporary particle in the grid that travelled to i, j.
     * @param readFrom State to read from
     * @param i   x coordinate
     * @param j   y coordinate
     * @param dt  the time between the two steps
     * @return    position to copy new value from
     */
   namespace mac{
   	glm::vec3 backTrackU(VelocityGrid const* const velocityGrid, int i, int j, int k, float dt);
   	glm::vec3 backTrackV(VelocityGrid const* const velocityGrid, int i, int j, int k, float dt);
   	glm::vec3 backTrackW(VelocityGrid const* const velocityGrid, int i, int j, int k, float dt);
   }
   
   glm::vec3 backTrack(VelocityGrid const* const velocityGrid, int i, int j, int k, float dt);
  }
} // util
#endif 
