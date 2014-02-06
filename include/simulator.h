template<typename T>
class OrdinalGrid;
template<typename T>
class Grid;
class State;

#include <glm/glm.hpp>

class Simulator {
public:
  Simulator(unsigned int width, unsigned int height);
  ~Simulator();

  void step(State *state, float dt);

private:
  void advecVelocity(OrdinalGrid<float> *fromVelocityGrid, OrdinalGrid<float> *toVelocityGrid);
  void calcGravity(OrdinalGrid<float> *fromVelocityGrid, OrdinalGrid<float> *toVelocityGrid, glm::vec2 g);
  void calcDivergence(OrdinalGrid<float> *fromVelocityGrid, OrdinalGrid<float> *toVelocityGrid);
  void jacobiIteration();
  void gradientSubtraction(OrdinalGrid<float> *fromVelocityGrid, OrdinalGrid<float> *toVelocityGrid);

  OrdinalGrid<double> *pressureGrid;
  unsigned int w, h;
};