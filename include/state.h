template<typename T>
class OrdinalGrid;
template<typename T>
class Grid;

class Simulator;

class State {
public:
  State(unsigned int width, unsigned int height);
  ~State();
  //  OrdinalGrid<double>const *const getPressureGrid() const;
  OrdinalGrid<float>const *const *const getVelocityGrid() const;
  //  void setPressureGrid(OrdinalGrid<double>*);
  void setVelocityGrid(OrdinalGrid<float>**);
  void setFluidGrid(Grid<bool>*);
  void setBoundaryGrid(Grid<bool>*);
  unsigned int getW();
  unsigned int getH();
private:
  //  OrdinalGrid<double> *pressureGrid;
  OrdinalGrid<float> **velocityGrid;
  Grid<bool> *fluidGrid;
  Grid<bool> *boundaryGrid;
  unsigned int w, h;


  friend class Simulator;
};
