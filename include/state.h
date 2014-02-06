template<typename T>
class OrdinalGrid;
template<typename T>
class Grid;

class State {
public:
  State(unsigned int width, unsigned int height);
  ~State();

private:
  OrdinalGrid<float> 
    *uVelocityGridPing, *uVelocityGridPong,
    *vVelocityGridPing, *vVelocityGridPong;

  unsigned int w, h;
};