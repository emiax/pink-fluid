template<typename T>
class OrdinalGrid;
template<typename T>
class Grid;
class State;

#include <glm/glm.hpp>

class Simulator {
public:
  Simulator(unsigned int width, unsigned int height);
  Simulator(State *statePing, State *statePong);
  ~Simulator();

  void step(State * const readFrom, State* writeTo, float dt);
  void step(float dt);

  glm::vec2 backTrack(State const * readFrom, int i,int j, float dt); 
  void advect(State const * readFrom, State * writeTo, float dt);

  void calcGravity(const OrdinalGrid<float> *fromVelocityGrid, OrdinalGrid<float> *toVelocityGrid, glm::vec2 g);
  void calcDivergence(const OrdinalGrid<float> *fromVelocityGrid, OrdinalGrid<float> *toVelocityGrid);
  void jacobiIteration();
  void gradientSubtraction(const OrdinalGrid<float> *fromVelocityGrid, OrdinalGrid<float> *toVelocityGrid);

private:
  unsigned int w,h;
  State *statePing, *statePong;
};
