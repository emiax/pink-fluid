template<typename T>
class OrdinalGrid;
template<typename T>
class Grid;

class Simulator;

class State {
public:
  State(unsigned int width, unsigned int height);
  ~State();
  OrdinalGrid<double>const *const getPressureGrid() const;
  OrdinalGrid<float>const *const *const getVelocityGrid() const;
  void setPressureGrid(OrdinalGrid<double>*);
  void setVelocityGrid(OrdinalGrid<float>**);
private:
  OrdinalGrid<double> *pressureGrid;
  OrdinalGrid<float> ** velocityGrid;
  unsigned int w, h;


  friend class Simulator;
};
