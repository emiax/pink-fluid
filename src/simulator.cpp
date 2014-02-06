#include <simulator.h>
#include <state.h>
#include <ordinalGrid.h>

Simulator::Simulator(unsigned int width, unsigned int height) : w(width), h(height) {
  pressureGrid = new OrdinalGrid<double>(w, h);
}

Simulator::~Simulator() {
  delete pressureGrid;
}

void Simulator::advecVelocity(OrdinalGrid<float> *fromVelocityGrid, OrdinalGrid<float> *toVelocityGrid) {
  
}