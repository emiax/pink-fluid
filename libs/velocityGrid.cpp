#include <velocityGrid.h>


VelocityGrid::VelocityGrid(unsigned int w, unsigned int h){
    u = new OrdinalGrid<float>(w+1,h);
    v = new OrdinalGrid<float>(w,h+1);
}
VelocityGrid::~VelocityGrid(){
  delete u;
  delete v;
}
glm::vec2 VelocityGrid::getCell(unsigned int i, unsigned int j) const{
  return glm::vec2(
    u->getLerp((float)i+0.5, j), 
    v->getLerp(i, (float)j+0.5)
  );
}
