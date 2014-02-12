template<typename T>
class OrdinalGrid;
template<typename T>
class Grid;

class Simulator;

struct VelocityGrid{
  VelocityGrid(int w, int h);
  ~VelocityGrid();
  OrdinalGrid<float> *u, *v; 
};

class State {
public:
  State(unsigned int width, unsigned int height);
  ~State();
  Grid<bool>const *const getBoundaryGrid() const;
  void setFluidGrid(Grid<bool> const* const);
  void setBoundaryGrid(Grid<bool> const* const);
  VelocityGrid const *const getVelocityGrid() const;
  void setVelocityGrid(VelocityGrid const* const);
  unsigned int getW();
  unsigned int getH();
private:
  //  OrdinalGrid<double> *pressureGrid;
  Grid<bool> *fluidGrid;
  Grid<bool> *boundaryGrid;
  void resetVelocityGrids();
  VelocityGrid *velocityGrid;
  unsigned int w, h;

  friend class Simulator;
};
