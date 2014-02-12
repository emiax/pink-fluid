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
  Grid<bool>const *const getBoundaryGrid() const;
  void setVelocityGrid(OrdinalGrid<float>const* const*);
  void setFluidGrid(Grid<bool> const* const);
  void setBoundaryGrid(Grid<bool> const* const);
  unsigned int getW();
  unsigned int getH();
private:
  //  OrdinalGrid<double> *pressureGrid;
  OrdinalGrid<float> **velocityGrid;
  Grid<bool> *fluidGrid;
  Grid<bool> *boundaryGrid;
  void resetVelocityGrids();
  unsigned int w, h;

  friend class Simulator;
};
