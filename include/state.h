template<typename T>
class OrdinalGrid;
template<typename T>
class Grid;

class Simulator;

class State {
public:
  State(unsigned int width, unsigned int height);
  ~State();
  OrdinalGrid<float>const *const *const getVelocityGrid() const;
  void setVelocityGrid(OrdinalGrid<float>**);
  unsigned int getW();
  unsigned int getH();
private:
  void resetVelocityGrids();
  OrdinalGrid<float> ** velocityGrid;
  unsigned int w, h;

  friend class Simulator;
};
