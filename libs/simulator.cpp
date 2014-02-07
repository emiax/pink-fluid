#include <simulator.h>
#include <state.h>
#include <ordinalGrid.h>

Simulator::Simulator(unsigned int width, unsigned int height) : w(width), h(height) {}

Simulator::~Simulator() {}

void Simulator::step(State * const readFrom, State* writeTo, float dt){
  advect(readFrom, writeTo, dt);
}

/**
 * Advects the velocity throughout the grid
 * @param readFrom State to read from, will remain constant
 * @param writeTo State to write to, will be modified after function call
 * @param dt time step length
 */
void Simulator::advect(State const* readFrom, State* writeTo, float dt){
  for(int i = 0; i <= w; i++){
    for(int j = 0; j < h; j++){
      glm::vec2 position = backTrack(readFrom, i, j, dt);
      writeTo->velocityGrid[0]->set(i,j,
                                    readFrom->velocityGrid[0]->getInterpolated(position));
    }
  }
  for(int i = 0; i < w; i++){
    for(int j = 0; j <= h; j++){
      glm::vec2 position = backTrack(readFrom, i, j, dt);
      writeTo->velocityGrid[1]->set(i,j,
                                    readFrom->velocityGrid[1]->getInterpolated(position));
    }
  } 
}


glm::vec2 Simulator::backTrack(State const* readFrom, int i, int j, float dt){
  glm::vec2 position(i,j);
  glm::vec2 v = glm::vec2(readFrom->velocityGrid[0]->get(i,j), 
                          readFrom->velocityGrid[1]->get(i,j));
  glm::vec2 midPos = position - (dt/2) * v;
  
  glm::vec2 midV = glm::vec2(readFrom->velocityGrid[0]->getInterpolated(midPos), 
                             readFrom->velocityGrid[1]->getInterpolated(midPos));
  return position-dt*midV;
}

