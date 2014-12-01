#include <ordinalGrid.h>
#include <velocityGrid.h>


VelocityGrid::VelocityGrid(unsigned int w, unsigned int h, unsigned int d){
  this->u = new OrdinalGrid<float>(w+1, h, d);
  this->v = new OrdinalGrid<float>(w, h+1, d);
  this->w = new OrdinalGrid<float>(w, h, d + 1);
}

VelocityGrid::~VelocityGrid(){
  delete u;
  delete v;
  delete w;
}

glm::vec3 VelocityGrid::getCell(unsigned int i, unsigned int j, unsigned int k) const{
  return glm::vec3(
    u->getLerp((float)i + 0.5, j, k),
    v->getLerp(i, (float)j + 0.5, k),
    w->getLerp(i, j, (float)k + 0.5)
  );
}

glm::vec3 VelocityGrid::getLerp(glm::vec3 p) const{
  return glm::vec3(
    u->getLerp(p.x + 0.5, p.y, p.z),
    v->getLerp(p.x, p.y + 0.5, p.z),
    w->getLerp(p.x, p.y, p.z + 0.5)
  );
}

std::ostream& VelocityGrid::write(std::ostream& stream){
  u->write(stream);
  v->write(stream);
  w->write(stream);
  return stream;
}

std::istream& VelocityGrid::read(std::istream& stream){
  u->read(stream);
  v->read(stream);
  w->read(stream);
  return stream;
}
