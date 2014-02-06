#include <state.h>
#include <ordinalGrid.h>

State::State(unsigned int width, unsigned int height) : w(width), h(height) {

  uVelocityGridPing = new OrdinalGrid<float>(w+1, h);
  uVelocityGridPong = new OrdinalGrid<float>(w+1, h);
  vVelocityGridPing = new OrdinalGrid<float>(w, h+1);
  vVelocityGridPong = new OrdinalGrid<float>(w, h+1);
}

State::~State() {
  delete uVelocityGridPing;
  delete uVelocityGridPong;
  delete vVelocityGridPing;
  delete vVelocityGridPong;
}