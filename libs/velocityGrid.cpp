#include <velocityGrid.h>


VelocityGrid::VelocityGrid(unsigned int w, unsigned int h){
    u = new OrdinalGrid<float>(w+1,h);
    v = new OrdinalGrid<float>(w,h+1);
    for(unsigned int i = 0u; i <= w; i++){
      for(unsigned int j = 0u; j < h; j++){
        u->set(i,j,0);
      }
    }
    for(unsigned int i = 0; i < w; i++){
      for(unsigned int j = 0; j <= h; j++){
        v->set(i,j,0);
      }
    }
}
VelocityGrid::~VelocityGrid(){
  delete u;
  delete v;
}
glm::vec2 VelocityGrid::getCell(unsigned int i, unsigned int j) const{
  return glm::vec2(
    u->getInterpolated((float)i+0.5, j), 
    v->getInterpolated(i, (float)j+0.5)
  );
}
