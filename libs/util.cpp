#include <util.h>
#include <velocityGrid.h>
namespace {
	inline glm::vec2 RK2BackTrack(VelocityGrid const* const velocityGrid, int i, int j, float dt, glm::vec2 dispU = glm::vec2(0), glm::vec2 dispV = glm::vec2(0)){
		glm::vec2 position(i,j);
		glm::vec2 v = glm::vec2(
			velocityGrid->u->getLerp(position + dispU),
			velocityGrid->v->getLerp(position + dispV)
			);
		glm::vec2 midPos = position - (dt/2) * v;
		glm::vec2 midV = glm::vec2(
			velocityGrid->u->getLerp(midPos),
			velocityGrid->v->getLerp(midPos)
			);
		return position-dt*midV;
	}
	inline glm::vec2 RK3BackTrack(VelocityGrid const* const velocityGrid, int i, int j, float dt, glm::vec2 dispU = glm::vec2(0), glm::vec2 dispV = glm::vec2(0)){
		glm::vec2 position(i,j);
		glm::vec2 k1 = glm::vec2(
			velocityGrid->u->getLerp(position + dispU),
			velocityGrid->v->getLerp(position + dispV)
			);

		glm::vec2 k2Inner = position - (dt/2) * k1;
		glm::vec2 k2 = glm::vec2(
			velocityGrid->u->getLerp(k2Inner + dispU),
			velocityGrid->v->getLerp(k2Inner + dispV)
			);

		glm::vec2 k3Inner = position + (3*dt/4) * k2;
		glm::vec2 k3 = glm::vec2(
			velocityGrid->u->getLerp(k3Inner + dispU),
			velocityGrid->v->getLerp(k3Inner + dispV)
			);

		return position - ((dt*2/9)*k1 + (dt*3/9)*k2 + (dt*4/9)*k3);
	}
}


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
 	glm::vec2 backTrackU(VelocityGrid const* const velocityGrid, int i, int j, float dt){
 		return RK3BackTrack(velocityGrid, i, j, dt, 
 			glm::vec2(0.0f),
 			glm::vec2(-0.5f, 0.5f));
 	}

 	glm::vec2 backTrackV(VelocityGrid const* const velocityGrid, int i, int j, float dt){
 		return RK3BackTrack(velocityGrid, i, j, dt, 
 			glm::vec2(0.5, -0.5),
 			glm::vec2(0.0f) 
 			);
 	}
 }
 

 glm::vec2 backTrack(VelocityGrid const* const velocityGrid, int i, int j, float dt){
 	return RK3BackTrack(velocityGrid, i, j, dt, 
 		glm::vec2(0.5,0), 
 		glm::vec2(0,0.5)
 		);
 }
}
} // util