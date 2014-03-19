#include <util.h>
#include <velocityGrid.h>
namespace {
	inline glm::vec3 RK2BackTrack(
		VelocityGrid const* const velocityGrid, 
		int i,
		int j,
		int k,
		float dt,
		glm::vec3 dispU = glm::vec3(0),
		glm::vec3 dispV = glm::vec3(0),
		glm::vec3 dispW = glm::vec3(0)){

		glm::vec3 position(i, j, k);
		glm::vec3 v = glm::vec3(
			velocityGrid->u->getLerp(position + dispU),
			velocityGrid->v->getLerp(position + dispV),
			velocityGrid->w->getLerp(position + dispW)
		);

		glm::vec3 midPos = position - (dt/2)*v;
		glm::vec3 midV = glm::vec3(
			velocityGrid->u->getLerp(midPos + dispU),
			velocityGrid->v->getLerp(midPos + dispV),
			velocityGrid->w->getLerp(midPos + dispW)
		);

		return position - dt*midV;
	}
	
	inline glm::vec3 RK3BackTrack(
		VelocityGrid const* const velocityGrid,
		int i,
		int j,
		int k,
		float dt,
		glm::vec3 dispU = glm::vec3(0),
		glm::vec3 dispV = glm::vec3(0),
		glm::vec3 dispW = glm::vec3(0)){

    glm::vec3 position(i, j, k);
		glm::vec3 k1 = glm::vec3(
			velocityGrid->u->getLerp(position + dispU),
			velocityGrid->v->getLerp(position + dispV),
			velocityGrid->w->getLerp(position + dispW)
		);

		glm::vec3 k2Inner = position - (dt/2)*k1;
		glm::vec3 k2 = glm::vec3(
			velocityGrid->u->getLerp(k2Inner + dispU),
			velocityGrid->v->getLerp(k2Inner + dispV),
			velocityGrid->w->getLerp(k2Inner + dispW)
		);

		glm::vec3 k3Inner = position + (3*dt/4) * k2;
		glm::vec3 k3 = glm::vec3(
			velocityGrid->u->getLerp(k3Inner + dispU),
			velocityGrid->v->getLerp(k3Inner + dispV),
			velocityGrid->w->getLerp(k3Inner + dispW)
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
		 	glm::vec3 backTrackU(VelocityGrid const* const velocityGrid, int i, int j, int k, float dt){
		 		return RK2BackTrack(velocityGrid, i, j, k, dt, 
		 			glm::vec3(0.0f),
		 			glm::vec3(-0.5f, 0.5f, 0.0f),
		 			glm::vec3(-0.5f, 0.0f, 0.5f)
	 			);
		 	}

		 	glm::vec3 backTrackV(VelocityGrid const* const velocityGrid, int i, int j, int k, float dt){
		 		return RK2BackTrack(velocityGrid, i, j, k, dt, 
		 			glm::vec3(0.5f, -0.5f, 0.0f),
		 			glm::vec3(0.0f),
		 			glm::vec3(0.0f, -0.5f, 0.5f)
	 			);
		 	}

		 	glm::vec3 backTrackW(VelocityGrid const* const velocityGrid, int i, int j, int k, float dt){
		 		return RK2BackTrack(velocityGrid, i, j, k, dt,
		 			glm::vec3(0.5f, 0.0f, -0.5f),
		 			glm::vec3(0.0f, 0.5f, -0.5f),
		 			glm::vec3(0.0f)
		 		);
		 	}
		} // mac

		glm::vec3 backTrack(VelocityGrid const* const velocityGrid, int i, int j, int k, float dt){
			return RK2BackTrack(velocityGrid, i, j, k, dt,
				glm::vec3(0.5f, 0.0f, 0.0f), 
				glm::vec3(0.0f, 0.5f, 0.0f),
				glm::vec3(0.0f, 0.0f, 0.5f)
			);
		}

	} // advect
} // util
